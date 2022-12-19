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

#include <sstream>

#include "call_manager_inner_type.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "state_registry_errors.h"
#include "string_ex.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "telephony_permission.h"
#include "telephony_state_registry_dump_helper.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
using namespace OHOS::EventFwk;
bool g_registerResult =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<TelephonyStateRegistryService>::GetInstance().get());

TelephonyStateRegistryService::TelephonyStateRegistryService()
    : SystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID, true)
{
    TELEPHONY_LOGI("TelephonyStateRegistryService SystemAbility create");
    slotSize_ = SIM_SLOT_COUNT;
}

TelephonyStateRegistryService::~TelephonyStateRegistryService()
{
    stateRecords_.clear();
    callState_.clear();
    callIncomingNumber_.clear();
    signalInfos_.clear();
    searchNetworkState_.clear();
}

void TelephonyStateRegistryService::OnStart()
{
    bindStartTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    std::lock_guard<std::mutex> guard(lock_);
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TELEPHONY_LOGE("Leave, FAILED, already running");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    bool ret = SystemAbility::Publish(DelayedSingleton<TelephonyStateRegistryService>::GetInstance().get());
    if (!ret) {
        TELEPHONY_LOGE("Leave, Failed to publish TelephonyStateRegistryService");
    }
    TELEPHONY_LOGI("TelephonyStateRegistryService start success.");
    bindEndTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (IsCommonEventServiceAbilityExist()) {
        for (int32_t i = 0; i < slotSize_; i++) {
            TELEPHONY_LOGI("TelephonyStateRegistryService send disconnected call state.");
            SendCallStateChanged(i, static_cast<int32_t>(TelCallState::CALL_STATUS_DISCONNECTED), u"");
        }
    }
}

void TelephonyStateRegistryService::OnStop()
{
    TELEPHONY_LOGI("TelephonyStateRegistryService OnStop ");
    std::lock_guard<std::mutex> guard(lock_);
    state_ = ServiceRunningState::STATE_STOPPED;
}

void TelephonyStateRegistryService::Finalize()
{
    TELEPHONY_LOGI("TelephonyStateRegistryService Finalize");
}

void TelephonyStateRegistryService::OnDump() {}

int32_t TelephonyStateRegistryService::UpdateCellularDataConnectState(
    int32_t slotId, int32_t dataState, int32_t networkType)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateCellularDataConnectState##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    cellularDataConnectionState_[slotId] = dataState;
    cellularDataConnectionNetworkType_[slotId] = networkType;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE) &&
            (record.slotId_ == slotId) && record.telephonyObserver_ != nullptr) {
            record.telephonyObserver_->OnCellularDataConnectStateUpdated(slotId, dataState, networkType);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendCellularDataConnectStateChanged(slotId, dataState, networkType);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCellularDataFlow(
    int32_t slotId, int32_t flowData)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateCellularDataFlow##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    cellularDataFlow_[slotId] = flowData;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW) &&
            (record.slotId_ == slotId) && record.telephonyObserver_ != nullptr) {
            record.telephonyObserver_->OnCellularDataFlowUpdated(slotId, flowData);
            result = TELEPHONY_SUCCESS;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCallState(
    int32_t slotId, int32_t callState, const std::u16string &number)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateCallState##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Check permission failed.");
        return TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) &&
            (record.slotId_ == slotId) && record.telephonyObserver_ != nullptr) {
            std::u16string phoneNumberStr;
            if (record.IsCanReadCallHistory()) {
                phoneNumberStr = number;
            } else {
                phoneNumberStr = Str8ToStr16("");
            }
            record.telephonyObserver_->OnCallStateUpdated(slotId, callState, phoneNumberStr);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendCallStateChanged(slotId, callState, number);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCallStateForSlotId(
    int32_t slotId, int32_t callId, int32_t callState, const std::u16string &incomingNumber)
{
    std::u16string incomingNumberStr = incomingNumber;
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateCallStateForSlotId##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE)) {
        TELEPHONY_LOGE("Check permission failed.");
        return TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED;
    }
    callState_[slotId] = callState;
    callIncomingNumber_[slotId] = incomingNumber;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) &&
            (record.slotId_ == slotId) && record.telephonyObserver_ != nullptr) {
            incomingNumberStr = GetCallIncomingNumberForSlotId(record, slotId);
            record.telephonyObserver_->OnCallStateUpdated(slotId, callState, incomingNumberStr);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendCallStateChanged(slotId, callState, incomingNumberStr);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateSimState(
    int32_t slotId, CardType type, SimState state, LockReason reason)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateSimState##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    simState_[slotId] = state;
    simReason_[slotId] = reason;
    cardType_[slotId] = type;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE) &&
            (record.slotId_ == slotId) && record.telephonyObserver_ != nullptr) {
            record.telephonyObserver_->OnSimStateUpdated(slotId, type, state, reason);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendSimStateChanged(slotId, type, state, reason);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateSignalInfo(
    int32_t slotId, const std::vector<sptr<SignalInformation>> &vec)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateSignalInfo##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    signalInfos_[slotId] = vec;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS) &&
            (record.slotId_ == slotId) && record.telephonyObserver_ != nullptr) {
            record.telephonyObserver_->OnSignalInfoUpdated(slotId, vec);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendSignalInfoChanged(slotId, vec);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCellInfo(
    int32_t slotId, const std::vector<sptr<CellInformation>> &vec)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateCellInfo##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    if (!TelephonyPermission::CheckPermission(Permission::SET_TELEPHONY_STATE) ||
        !TelephonyPermission::CheckPermission(Permission::CELL_LOCATION)) {
        TELEPHONY_LOGE("Check permission failed.");
        return TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED;
    }
    cellInfos_[slotId] = vec;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO) &&
            record.slotId_ == slotId) {
            if (record.telephonyObserver_ == nullptr) {
                TELEPHONY_LOGE("record.telephonyObserver_ is nullptr");
                return TELEPHONY_ERR_LOCAL_PTR_NULL;
            }
            record.telephonyObserver_->OnCellInfoUpdated(slotId, vec);
            result = TELEPHONY_SUCCESS;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::UpdateNetworkState(
    int32_t slotId, const sptr<NetworkState> &networkState)
{
    if (!VerifySlotId(slotId)) {
        TELEPHONY_LOGE(
            "UpdateNetworkState##VerifySlotId failed ##slotId = %{public}d", slotId);
        return TELEPHONY_STATE_REGISTRY_SLODID_ERROR;
    }
    searchNetworkState_[slotId] = networkState;
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord r = stateRecords_[i];
        if (r.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) &&
            (r.slotId_ == slotId) && r.telephonyObserver_ != nullptr) {
            r.telephonyObserver_->OnNetworkStateUpdated(slotId, networkState);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendNetworkStateChanged(slotId, networkState);
    TELEPHONY_LOGI("TelephonyStateRegistryService::NotifyNetworkStateUpdated end");
    return result;
}

int32_t TelephonyStateRegistryService::RegisterStateChange(
    const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t slotId, uint32_t mask, const std::string &bundleName, bool isUpdate, pid_t pid)
{
    if (!CheckPermission(mask)) {
        return TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED;
    }
    std::lock_guard<std::mutex> guard(lock_);
    bool isExist = false;
    TelephonyStateRegistryRecord record;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        record = stateRecords_[i];
        if (record.slotId_ == slotId && record.mask_ == mask && record.pid_ == pid) {
            isExist = true;
            break;
        }
    }

    if (!isExist) {
        record.pid_ = pid;
        record.slotId_ = slotId;
        record.mask_ = mask;
        record.bundleName_ = bundleName;
        record.telephonyObserver_ = telephonyObserver;
        stateRecords_.push_back(record);
    }

    if (isUpdate && VerifySlotId(slotId)) {
        UpdateData(record);
    }
    return TELEPHONY_SUCCESS;
}

int32_t TelephonyStateRegistryService::UnregisterStateChange(
    int32_t slotId, uint32_t mask, pid_t pid)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST;
    std::vector<TelephonyStateRegistryRecord>::iterator it;
    for (it = stateRecords_.begin(); it != stateRecords_.end(); ++it) {
        if (it->slotId_ == slotId && it->mask_ == mask && it->pid_ == pid) {
            stateRecords_.erase(it);
            result = TELEPHONY_SUCCESS;
            break;
        }
    }
    return result;
}

bool TelephonyStateRegistryService::CheckPermission(uint32_t mask)
{
    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) != 0) {
        if (!TelephonyPermission::CheckPermission(Permission::GET_NETWORK_INFO)) {
            TELEPHONY_LOGE("Check permission failed,"
                " you must declare ohos.permission.GET_NETWORK_INFO permission for network state");
            return false;
        }
    }
    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO) != 0) {
        if (!TelephonyPermission::CheckPermission(Permission::CELL_LOCATION)) {
            TELEPHONY_LOGE("Check permission failed,"
                " you must declare ohos.permission.LOCATION permission for cell info");
            return false;
        }
    }
    return true;
}

bool TelephonyStateRegistryService::VerifySlotId(int slotId)
{
    return slotId >= 0 && slotId < slotSize_;
}

std::u16string TelephonyStateRegistryService::GetCallIncomingNumberForSlotId(
    TelephonyStateRegistryRecord record, int32_t slotId)
{
    if (record.IsCanReadCallHistory()) {
        return callIncomingNumber_[slotId];
    } else {
        return Str8ToStr16("");
    }
}

void TelephonyStateRegistryService::UpdateData(const TelephonyStateRegistryRecord &record)
{
    if (record.telephonyObserver_ == nullptr) {
        TELEPHONY_LOGE("record.telephonyObserver_ is nullptr");
        return;
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) != 0) {
        std::u16string phoneNumber = GetCallIncomingNumberForSlotId(record, record.slotId_);
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_CALL_STATE");
        record.telephonyObserver_->OnCallStateUpdated(
            record.slotId_, callState_[record.slotId_], phoneNumber);
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS) != 0) {
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_SIGNAL_STRENGTHS");
        record.telephonyObserver_->OnSignalInfoUpdated(
            record.slotId_, signalInfos_[record.slotId_]);
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) != 0) {
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_NETWORK_STATE");
        record.telephonyObserver_->OnNetworkStateUpdated(
            record.slotId_, searchNetworkState_[record.slotId_]);
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO) != 0) {
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_CELL_INFO");
        record.telephonyObserver_->OnCellInfoUpdated(
            record.slotId_, cellInfos_[record.slotId_]);
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE) != 0) {
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_SIM_STATE");
        record.telephonyObserver_->OnSimStateUpdated(
            record.slotId_, cardType_[record.slotId_], simState_[record.slotId_], simReason_[record.slotId_]);
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE) != 0) {
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_DATA_CONNECTION_STATE");
        record.telephonyObserver_->OnCellularDataConnectStateUpdated(
            record.slotId_, cellularDataConnectionState_[record.slotId_],
            cellularDataConnectionNetworkType_[record.slotId_]);
    }
    if ((record.mask_ & TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW) != 0) {
        TELEPHONY_LOGI("RegisterStateChange##Notify-OBSERVER_MASK_DATA_FLOW");
        record.telephonyObserver_->OnCellularDataFlowUpdated(
            record.slotId_, cellularDataFlow_[record.slotId_]);
    }
}

bool TelephonyStateRegistryService::PublishCommonEvent(
    const AAFwk::Want &want, int eventCode, const std::string &eventData)
{
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    TELEPHONY_LOGI("PublishCommonEvent end###publishResult = %{public}d\n", publishResult);
    return publishResult;
}

void TelephonyStateRegistryService::SendCallStateChanged(
    int32_t slotId, int32_t state, const std::u16string &number)
{
    AAFwk::Want want;
    want.SetParam("slotId", slotId);
    want.SetParam("state", state);
    want.SetParam("number", Str16ToStr8(number));
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CALL_STATE_CHANGED);

    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    std::vector<std::string> callPermissions;
    callPermissions.emplace_back(Permission::GET_TELEPHONY_STATE);
    publishInfo.SetSubscriberPermissions(callPermissions);
    bool publishResult = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    if (!publishResult) {
        TELEPHONY_LOGE("SendCallStateChanged PublishBroadcastEvent result fail");
    }
}

void TelephonyStateRegistryService::SendCellularDataConnectStateChanged(
    int32_t slotId, int32_t dataState, int32_t networkType)
{
    AAFwk::Want want;
    want.SetParam("slotId", slotId);
    want.SetParam("dataState", dataState);
    want.SetParam("networkType", networkType);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_CELLULAR_DATA_STATE_CHANGED);
    int32_t eventCode = 1;
    std::string eventData("connectStateChanged");
    PublishCommonEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendSimStateChanged(
    int32_t slotId, CardType type, SimState state, LockReason reason)
{
    AAFwk::Want want;
    want.SetParam("slotId", slotId);
    want.SetParam("cardType", static_cast<int32_t>(type));
    want.SetParam("reason", static_cast<int32_t>(reason));
    want.SetParam("state", static_cast<int32_t>(state));
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIM_STATE_CHANGED);
    int32_t eventCode = 1;
    std::string eventData("simStateChanged");
    PublishCommonEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendSignalInfoChanged(
    int32_t slotId, const std::vector<sptr<SignalInformation>> &vec)
{
    AAFwk::Want want;
    want.SetParam("slotId", slotId);
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SIGNAL_INFO_CHANGED);
    std::vector<std::string> contentStr;
    for (size_t i = 0; i < vec.size(); i++) {
        sptr<SignalInformation> signal = vec[i];
        if (signal != nullptr) {
            contentStr.push_back(signal->ToString());
        }
    }
    want.SetParam("signalInfos", contentStr);
    int32_t eventCode = 1;
    std::string eventData("signalInfoChanged");
    PublishCommonEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendNetworkStateChanged(int32_t slotId, const sptr<NetworkState> &networkState)
{
    AAFwk::Want want;
    want.SetParam("slotId", slotId);
    want.SetAction(
        EventFwk::CommonEventSupport::COMMON_EVENT_NETWORK_STATE_CHANGED);
    int32_t eventCode = 1;
    if (networkState != nullptr) {
        want.SetParam("networkState", networkState->ToString());
    }
    std::string eventData("networkStateChanged");
    PublishCommonEvent(want, eventCode, eventData);
}

int TelephonyStateRegistryService::Dump(std::int32_t fd, const std::vector<std::u16string> &args)
{
    if (fd < 0) {
        TELEPHONY_LOGE("dump fd invalid");
        return TELEPHONY_ERR_FAIL;
    }
    std::vector<std::string> argsInStr;
    for (const auto &arg : args) {
        TELEPHONY_LOGI("Dump args: %{public}s", Str16ToStr8(arg).c_str());
        argsInStr.emplace_back(Str16ToStr8(arg));
    }
    std::string result;
    TelephonyStateRegistryDumpHelper dumpHelper;
    if (dumpHelper.Dump(argsInStr, stateRecords_, result)) {
        std::int32_t ret = dprintf(fd, "%s", result.c_str());
        if (ret < 0) {
            TELEPHONY_LOGE("dprintf to dump fd failed");
            return TELEPHONY_ERR_FAIL;
        }
        return TELEPHONY_SUCCESS;
    }
    TELEPHONY_LOGW("dumpHelper failed");
    return TELEPHONY_ERR_FAIL;
}

std::string TelephonyStateRegistryService::GetBindStartTime()
{
    std::ostringstream oss;
    oss << bindStartTime_;
    return oss.str();
}

std::string TelephonyStateRegistryService::GetBindEndTime()
{
    std::ostringstream oss;
    oss << bindEndTime_;
    return oss.str();
}

std::string TelephonyStateRegistryService::GetBindSpendTime()
{
    std::ostringstream oss;
    oss << (bindEndTime_ - bindStartTime_);
    return oss.str();
}

int32_t TelephonyStateRegistryService::GetServiceRunningState()
{
    return static_cast<int32_t>(state_);
}

int32_t TelephonyStateRegistryService::GetSimState(int32_t slotId)
{
    std::map<int32_t, SimState>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = simState_.begin(); it != simState_.end(); ++it) {
        if (it->first == slotId) {
            result = static_cast<int32_t>(it->second);
            TELEPHONY_LOGI("CallState = %{public}d", result);
            break;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::GetCallState(int32_t slotId)
{
    std::map<int32_t, int32_t>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = callState_.begin(); it != callState_.end(); ++it) {
        if (it->first == slotId) {
            result = it->second;
            break;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::GetCardType(int32_t slotId)
{
    std::map<int32_t, CardType>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = cardType_.begin(); it != cardType_.end(); ++it) {
        if (it->first == slotId) {
            result = static_cast<int32_t>(it->second);
            break;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::GetCellularDataConnectionState(int32_t slotId)
{
    std::map<int32_t, int32_t>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = cellularDataConnectionState_.begin(); it != cellularDataConnectionState_.end(); ++it) {
        if (it->first == slotId) {
            result = it->second;
            break;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::GetCellularDataFlow(int32_t slotId)
{
    std::map<int32_t, int32_t>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = cellularDataFlow_.begin(); it != cellularDataFlow_.end(); ++it) {
        if (it->first == slotId) {
            result = it->second;
            break;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::GetCellularDataConnectionNetworkType(int32_t slotId)
{
    std::map<int32_t, int32_t>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = cellularDataConnectionNetworkType_.begin(); it != cellularDataConnectionNetworkType_.end(); ++it) {
        if (it->first == slotId) {
            result = it->second;
            break;
        }
    }
    return result;
}

int32_t TelephonyStateRegistryService::GetLockReason(int32_t slotId)
{
    std::map<int32_t, LockReason>::iterator it;
    int32_t result = TELEPHONY_ERROR;
    for (it = simReason_.begin(); it != simReason_.end(); ++it) {
        if (it->first == slotId) {
            result = static_cast<int32_t>(it->second);
            break;
        }
    }
    return result;
}

bool TelephonyStateRegistryService::IsCommonEventServiceAbilityExist()
{
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        TELEPHONY_LOGE("IsCommonEventServiceAbilityExist Get ISystemAbilityManager failed, no SystemAbilityManager");
        return false;
    }
    sptr<IRemoteObject> remote = sm->CheckSystemAbility(COMMON_EVENT_SERVICE_ID);
    if (remote == nullptr) {
        TELEPHONY_LOGE("No CesServiceAbility");
        return false;
    }
    return true;
}
} // namespace Telephony
} // namespace OHOS
