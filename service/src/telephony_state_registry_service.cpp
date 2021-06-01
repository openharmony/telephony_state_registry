/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "telephony_state_registry_service.h"
#include "system_ability_definition.h"

#include "string_ex.h"
#include "telephony_errors.h"

namespace OHOS {
namespace TelephonyState {
bool g_registerResult =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<TelephonyStateRegistryService>::GetInstance().get());

TelephonyStateRegistryService::TelephonyStateRegistryService()
    : SystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID, true)
{
    HILOG_DEBUG("TelephonyStateRegistryService SystemAbility create");
    state_ = ServiceRunningState::STATE_STOPPED;
    slotSize_ = 0;
    RegistSubscriber();
}

TelephonyStateRegistryService::~TelephonyStateRegistryService()
{
    UnRegistSubscriber();
    stateRecords_.clear();
    callState_.clear();
    callIncomingNumber_.clear();
    signalInformations_.clear();
    searchNetworkState_.clear();
}

void TelephonyStateRegistryService::OnStart()
{
    HILOG_DEBUG("TelephonyStateRegistryService OnStart");
    std::lock_guard<std::mutex> guard(lock_);
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        HILOG_ERROR("Leave, FAILED, already running");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    Init();
    bool ret = SystemAbility::Publish(DelayedSingleton<TelephonyStateRegistryService>::GetInstance().get());
    if (!ret) {
        HILOG_ERROR("Leave, Failed to publish TelephonyStateRegistryService");
    }
    HILOG_DEBUG("TelephonyStateRegistryService start success.");
}

void TelephonyStateRegistryService::OnStop()
{
    HILOG_DEBUG("TelephonyStateRegistryService OnStop ");
    std::lock_guard<std::mutex> guard(lock_);
    state_ = ServiceRunningState::STATE_STOPPED;
}

bool TelephonyStateRegistryService::Init()
{
    HILOG_DEBUG("TelephonyStateRegistryService SystemAbility Init start------------");
    int32_t numPhons = 1;
    slotSize_ = numPhons;
    for (int i = 0; i < slotSize_; i++) {
        callState_.push_back(0);
        callIncomingNumber_.push_back(Str8ToStr16(""));
    }
    return true;
}

void TelephonyStateRegistryService::Finalize()
{
    HILOG_DEBUG("TelephonyStateRegistryService Finalize");
}

void TelephonyStateRegistryService::OnDump()
{
    HILOG_DEBUG("TelephonyStateRegistryService OnDump");
}

int32_t TelephonyStateRegistryService::UpdateCallState(int32_t callState, const std::u16string &number)
{
    std::lock_guard<std::mutex> guard(lock_);
    std::string utf8 = Str16ToStr8(number);
    int32_t result = TELEPHONY_NO_ERROR;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE)) {
            std::u16string phoneNumberStr;
            if (record.IsCanReadCallHistory()) {
                phoneNumberStr = number;
            } else {
                phoneNumberStr = Str8ToStr16("");
            }
            utf8 = Str16ToStr8(phoneNumberStr);
            record.telephonyObserver_->OnCallStateUpdated(callState, phoneNumberStr);
        }
    }
    SendCallStateChanged(0, 0, callState, number);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCallStateForSlotIndex(
    int32_t simId, int32_t slotIndex, int32_t callState, const std::u16string &incomingNumber)
{
    std::lock_guard<std::mutex> guard(lock_);
    std::string utf8 = Str16ToStr8(incomingNumber);
    int32_t result = TELEPHONY_NO_ERROR;
    if (VerifySlotIndex(slotIndex)) {
        callState_[slotIndex] = callState;
        callIncomingNumber_[slotIndex] = incomingNumber;
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) &&
                (record.simId_ == simId)) {
                std::u16string incomingNumberStr = getCallIncomingNumberForSlotIndex(record, slotIndex);
                record.telephonyObserver_->OnCallStateUpdated(callState, incomingNumberStr);
            }
        }
    }
    SendCallStateChanged(simId, slotIndex, callState, incomingNumber);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateSignalInfo(
    int32_t simId, int32_t slotIndex, const std::vector<sptr<SignalInformation>> &vec)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_NO_ERROR;
    if (VerifySlotIndex(slotIndex)) {
        signalInformations_[slotIndex] = vec;
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS) &&
                IsSameId(record.simId_, simId, slotIndex)) {
                record.telephonyObserver_->OnSignalInfoUpdated(vec);
            }
        }
    }
    SendSignalInfoChanged(simId, slotIndex, vec);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateNetworkState(
    int32_t simId, int32_t slotIndex, const sptr<NetworkState> &networkState)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_NO_ERROR;
    if (VerifySlotIndex(slotIndex)) {
        searchNetworkState_[slotIndex] = networkState;

        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord r = stateRecords_[i];
            if (r.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) &&
                IsSameId(r.simId_, simId, slotIndex)) {
                r.telephonyObserver_->OnNetworkStateUpdated(networkState);
            }
        }
    }
    SendNetworkStateChanged(simId, slotIndex, networkState);
    HILOG_DEBUG("TelephonyStateRegistryService::NotifyNetworkStateUpdated end");
    return result;
}

int32_t TelephonyStateRegistryService::RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t simId, uint32_t mask, const std::u16string &package, bool isUpdate, pid_t pid)
{
    std::lock_guard<std::mutex> guard(lock_);
    std::string utf8 = Str16ToStr8(package);
    HILOG_DEBUG("TelephonyStateRegistryService::RegisterStateChange %{public}s", utf8.c_str());
    int32_t result = TELEPHONY_NO_ERROR;
    bool isExist = false;
    TelephonyStateRegistryRecord record;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        record = stateRecords_[i];
        if (record.simId_ == simId && record.mask_ == mask && record.pid_ == pid) {
            isExist = true;
            break;
        }
    }

    int32_t slotIndex = 0;
    if (!isExist) {
        record.pid_ = pid;
        record.simId_ = simId;
        record.slotIndex_ = slotIndex;
        record.mask_ = mask;
        record.package_ = package;
        record.telephonyObserver_ = telephonyObserver;
        stateRecords_.push_back(record);
    }

    bool isNull = false;
    if (stateRecords_.size() > 0) {
        isNull = (stateRecords_[stateRecords_.size() - 1].telephonyObserver_ != nullptr);
    }
    if (isUpdate && VerifySlotIndex(slotIndex)) {
        UpdateData(record, mask, slotIndex);
    }

    return result;
}

int32_t TelephonyStateRegistryService::UnregisterStateChange(int32_t simId, uint32_t mask, pid_t pid)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_NO_ERROR;
    std::vector<TelephonyStateRegistryRecord>::iterator it;
    for (it = stateRecords_.begin(); it != stateRecords_.end(); ++it) {
        if (it->simId_ == simId && it->mask_ == mask && it->pid_ == pid) {
            stateRecords_.erase(it);
            break;
        }
    }
    return result;
}

bool TelephonyStateRegistryService::VerifySlotIndex(int slotIndex)
{
    bool valid = (slotIndex >= 0) && (slotIndex < slotSize_);
    return valid;
}

std::u16string TelephonyStateRegistryService::getCallIncomingNumberForSlotIndex(
    TelephonyStateRegistryRecord record, int32_t slotIndex)
{
    if (record.IsCanReadCallHistory()) {
        return callIncomingNumber_[slotIndex];
    } else {
        return Str8ToStr16("");
    }
}

bool TelephonyStateRegistryService::IsSameId(int rSimId, int simId, int slotIndex)
{
    return (rSimId == simId);
}

void TelephonyStateRegistryService::UpdateData(
    const TelephonyStateRegistryRecord &record, uint32_t mask, int32_t slotIndex)
{
    HILOG_DEBUG("TelephonyStateRegistryService::RegisterStateChange##Notify start");
    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) != 0) {
        std::u16string phoneNumber = getCallIncomingNumberForSlotIndex(record, slotIndex);
        HILOG_DEBUG("TelephonyStateRegistryService::RegisterStateChange##Notify-LISTEN_CALL_STATE");
        record.telephonyObserver_->OnCallStateUpdated(callState_[slotIndex], phoneNumber);
    }
    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS) != 0) {
        HILOG_DEBUG("TelephonyStateRegistryService::RegisterStateChange##Notify-LISTEN_SIGNAL_STRENGTHS");
        record.telephonyObserver_->OnSignalInfoUpdated(signalInformations_[slotIndex]);
    }

    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) != 0) {
        HILOG_DEBUG("TelephonyStateRegistryService::RegisterStateChange##Notify-LISTEN_NET_WORK_STATE");
        record.telephonyObserver_->OnNetworkStateUpdated(searchNetworkState_[slotIndex]);
    }
}

bool TelephonyStateRegistryService::PublishSimFileEvent(
    const AAFwk::Want &want, int eventCode, const std::string &eventData)
{
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    HILOG_DEBUG("PublishSimFileEvent end###publishResult = %{public}d\n", publishResult);
    return publishResult;
}

void TelephonyStateRegistryService::UnRegistSubscriber()
{
    HILOG_DEBUG("UnRegistSubscriber start.\n");
    if (stateSubscriber_ != nullptr) {
        bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(stateSubscriber_);
        HILOG_DEBUG("UnRegistSubscriber end###subscribeResult = %{public}d\n", subscribeResult);
    }
    HILOG_DEBUG("UnRegistSubscriber end..\n");
}

void TelephonyStateRegistryService::RegistSubscriber()
{
    HILOG_DEBUG("RegistSubscriber start.\n");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SPN_INFO_UPDATED_ACTION);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    stateSubscriber_ = std::make_shared<StateSubscriber>(subscriberInfo);
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(stateSubscriber_);
    HILOG_DEBUG("RegistSubscriber end###subscribeResult = %{public}d\n", subscribeResult);
}

void StateSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    AAFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    HILOG_DEBUG("Subscriber::OnReceiveEvent action = %{public}s\n", action.c_str());

    int msgcode = GetCode();
    std::string msgdata = GetData();
    HILOG_DEBUG("Subscriber::OnReceiveEvent msgdata = %{public}s,msgcode = %{public}d\n", msgdata.c_str(), msgcode);
}
void TelephonyStateRegistryService::SendCallStateChanged(
    int32_t simId, int32_t slotIndex, int32_t state, const std::u16string &number)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetParam("slotIndex", slotIndex);
    want.SetParam("state", state);
    want.SetParam("number", Str16ToStr8(number));
    want.SetAction(CALL_STATE_CHANGE_ACTION);
    int32_t eventCode = 1;
    std::string eventData("callStateChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendSignalInfoChanged(
    int32_t simId, int32_t slotIndex, const std::vector<sptr<SignalInformation>> &vec)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetParam("slotIndex", slotIndex);
    want.SetAction(SEARCH_SIGNAL_INFO_CHANGE_ACTION);
    int32_t eventCode = 1;
    std::string eventData("signalInfoChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendNetworkStateChanged(
    int32_t simId, int32_t slotIndex, const sptr<NetworkState> &networkState)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetParam("slotIndex", slotIndex);
    want.SetAction(SEARCH_NET_WORK_STATE_CHANGE_ACTION);
    int32_t eventCode = 1;
    std::string eventData("networkStateChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}
} // namespace TelephonyState
} // namespace OHOS