/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <new>

#include "napi_radio_types.h"
#include "observer_event_handler.h"
#include "securec.h"
#include "telephony_errors.h"
#include "telephony_observer_impl.h"
#include "telephony_state_manager.h"

namespace OHOS {
namespace Telephony {
namespace {
int32_t WrapRegState(int32_t nativeState)
{
    RegServiceState state = static_cast<RegServiceState>(nativeState);
    switch (state) {
        case RegServiceState::REG_STATE_NO_SERVICE:
        case RegServiceState::REG_STATE_SEARCH: {
            return RegStatus::REGISTRATION_STATE_NO_SERVICE;
        }
        case RegServiceState::REG_STATE_IN_SERVICE: {
            return RegStatus::REGISTRATION_STATE_IN_SERVICE;
        }
        case RegServiceState::REG_STATE_EMERGENCY_ONLY: {
            return RegStatus::REGISTRATION_STATE_EMERGENCY_CALL_ONLY;
        }
        case RegServiceState::REG_STATE_UNKNOWN: {
            return RegStatus::REGISTRATION_STATE_POWER_OFF;
        }
        default:
            return RegStatus::REGISTRATION_STATE_POWER_OFF;
    }
}

int32_t WrapCallState(int32_t callState)
{
    switch (callState) {
        case (int32_t)Telephony::CallStatus::CALL_STATUS_ACTIVE:
        case (int32_t)Telephony::CallStatus::CALL_STATUS_HOLDING:
        case (int32_t)Telephony::CallStatus::CALL_STATUS_DIALING:
        case (int32_t)Telephony::CallStatus::CALL_STATUS_ALERTING:
            return static_cast<int32_t>(CallState::CALL_STATE_OFFHOOK);
        case (int32_t)Telephony::CallStatus::CALL_STATUS_WAITING:
        case (int32_t)Telephony::CallStatus::CALL_STATUS_INCOMING:
            return static_cast<int32_t>(CallState::CALL_STATE_RINGING);
        case (int32_t)Telephony::CallStatus::CALL_STATUS_DISCONNECTING:
        case (int32_t)Telephony::CallStatus::CALL_STATUS_DISCONNECTED:
        case (int32_t)Telephony::CallStatus::CALL_STATUS_IDLE:
            return static_cast<int32_t>(CallState::CALL_STATE_IDLE);
        case (int32_t)Telephony::CallStatus::CALL_STATUS_ANSWERED:
            return static_cast<int32_t>(CallState::CALL_STATE_ANSWERED);
        default:
            return static_cast<int32_t>(CallState::CALL_STATE_UNKNOWN);
    }
}

int32_t WrapNetworkType(SignalInformation::NetworkType nativeNetworkType)
{
    NetworkType jsNetworkType = NetworkType::NETWORK_TYPE_UNKNOWN;
    switch (nativeNetworkType) {
        case SignalInformation::NetworkType::GSM: {
            jsNetworkType = NetworkType::NETWORK_TYPE_GSM;
            break;
        }
        case SignalInformation::NetworkType::CDMA: {
            jsNetworkType = NetworkType::NETWORK_TYPE_CDMA;
            break;
        }
        case SignalInformation::NetworkType::LTE: {
            jsNetworkType = NetworkType::NETWORK_TYPE_LTE;
            break;
        }
        case SignalInformation::NetworkType::TDSCDMA: {
            jsNetworkType = NetworkType::NETWORK_TYPE_TDSCDMA;
            break;
        }
        case SignalInformation::NetworkType::WCDMA: {
            jsNetworkType = NetworkType::NETWORK_TYPE_WCDMA;
            break;
        }
        default: {
            jsNetworkType = NetworkType::NETWORK_TYPE_UNKNOWN;
        }
    }
    return static_cast<int32_t>(jsNetworkType);
}

int32_t WrapRadioTech(int32_t radioTechType)
{
    RadioTech techType = static_cast<RadioTech>(radioTechType);
    switch (techType) {
        case RadioTech::RADIO_TECHNOLOGY_GSM:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_GSM);
        case RadioTech::RADIO_TECHNOLOGY_LTE:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_LTE);
        case RadioTech::RADIO_TECHNOLOGY_WCDMA:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_WCDMA);
        case RadioTech::RADIO_TECHNOLOGY_1XRTT:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_1XRTT);
        case RadioTech::RADIO_TECHNOLOGY_HSPA:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_HSPA);
        case RadioTech::RADIO_TECHNOLOGY_HSPAP:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_HSPAP);
        case RadioTech::RADIO_TECHNOLOGY_TD_SCDMA:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_TD_SCDMA);
        case RadioTech::RADIO_TECHNOLOGY_EVDO:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_EVDO);
        case RadioTech::RADIO_TECHNOLOGY_EHRPD:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_EHRPD);
        case RadioTech::RADIO_TECHNOLOGY_LTE_CA:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_LTE_CA);
        case RadioTech::RADIO_TECHNOLOGY_IWLAN:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_IWLAN);
        case RadioTech::RADIO_TECHNOLOGY_NR:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_NR);
        case RadioTech::RADIO_TECHNOLOGY_NR_ENHANCED:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_NR_ENHANCED);
        default:
            return static_cast<int32_t>(RatType::RADIO_TECHNOLOGY_UNKNOWN);
    }
}
} // namespace

std::mutex ObserverEventHandler::operatorMutex_;

ObserverEventHandler::ObserverEventHandler() : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create())
{
}

ObserverEventHandler::~ObserverEventHandler()
{
    listenerList_.clear();
}

void ObserverEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    auto eventId = static_cast<TelephonyCallbackEventId>(event->GetInnerEventId());
    switch (eventId) {
        case TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE:
            HandleCallbackInfoUpdate<CallStateUpdateInfo,
                TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE>(event);
                break;
        case TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE:
            HandleCallbackInfoUpdate<SimStateUpdateInfo,
                TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE:
            HandleCallbackInfoUpdate<CellularDataConnectState,
                TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_FLOW_UPDATE:
            HandleCallbackInfoUpdate<CellularDataFlowUpdate,
                TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_CFU_INDICATOR_UPDATE:
            HandleCallbackInfoUpdate<CfuIndicatorUpdate,
                TelephonyUpdateEventType::EVENT_CFU_INDICATOR_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_VOICE_MAIL_MSG_INDICATOR_UPDATE:
            HandleCallbackInfoUpdate<VoiceMailMsgIndicatorUpdate,
                TelephonyUpdateEventType::EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_ICC_ACCOUNT_UPDATE:
            HandleCallbackVoidUpdate<TelephonyUpdateEventType::EVENT_ICC_ACCOUNT_CHANGE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE:
            HandleCallbackInfoUpdate<SignalUpdateInfo,
                TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE:
            HandleCallbackInfoUpdate<NetworkStateUpdateInfo,
                TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE>(event);
            break;
        case TelephonyCallbackEventId::EVENT_ON_CELL_INFOMATION_UPDATE:
            HandleCallbackInfoUpdate<CellInfomationUpdate,
                TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE>(event);
            break;
        default:
            TELEPHONY_LOGE("ObserverEventHandler::ProcessEvent Unkonw Telephony CallbackEventId");
            return;
    }
}

int32_t ObserverEventHandler::CheckEventListenerRegister(EventListener &eventListener)
{
    int32_t flag = EVENT_LISTENER_DIFF;
    for (auto &listen : listenerList_) {
        if (eventListener.slotId == listen.slotId &&
            eventListener.eventType == listen.eventType &&
            eventListener.funcId == listen.funcId) {
            flag = EVENT_LISTENER_SAME;
            return flag;
        }
        if (eventListener.slotId == listen.slotId && eventListener.eventType == listen.eventType) {
            flag = EVENT_LISTENER_SLOTID_AND_EVENTTYPE_SAME;
        }
    }
    return flag;
}

int32_t ObserverEventHandler::RegisterEventListener(EventListener &eventListener)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    int32_t registerStatus = CheckEventListenerRegister(eventListener);
    if (registerStatus == EVENT_LISTENER_SAME) {
        TELEPHONY_LOGE(" ObserverEventHandler::RegisterEventListener CALLBACK ALREADY REGISTERED");
        return TELEPHONY_ERR_CALLBACK_ALREADY_REGISTERED;
    }
    if (registerStatus != EVENT_LISTENER_SLOTID_AND_EVENTTYPE_SAME) {
        FfiTelephonyObserver *telephonyObserver = std::make_unique<FfiTelephonyObserver>().release();
        if (telephonyObserver == nullptr) {
            TELEPHONY_LOGE("error by telephonyObserver nullptr");
            return TELEPHONY_ERR_LOCAL_PTR_NULL;
        }
        sptr<TelephonyObserverBroker> observer(telephonyObserver);
        if (observer == nullptr) {
            TELEPHONY_LOGE("error by observer nullptr");
            return TELEPHONY_ERR_LOCAL_PTR_NULL;
        }
        int32_t addResult = TelephonyStateManager::AddStateObserver(
            observer, eventListener.slotId, static_cast<uint32_t>(eventListener.eventType),
            eventListener.eventType == TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE);
        if (addResult != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("AddStateObserver failed, ret=%{public}d!", addResult);
            return addResult;
        }
    }
    listenerList_.push_back(eventListener);
    TELEPHONY_LOGI("ObserverEventHandler::RegisterEventListener listenerList_ size=%{public}d",
        static_cast<int32_t>(listenerList_.size()));
    return TELEPHONY_SUCCESS;
}

void ObserverEventHandler::SetEventListenerDeleting(std::shared_ptr<bool> isDeleting)
{
    if (isDeleting == nullptr) {
        TELEPHONY_LOGE("isDeleting is nullptr");
        return;
    }
    *isDeleting = true;
}

void ObserverEventHandler::RemoveEventListenerRegister(const TelephonyUpdateEventType eventType, int64_t funcId,
    std::set<int32_t> &soltIdSet)
{
    std::list<EventListener>::iterator it = listenerList_.begin();
    while (it != listenerList_.end()) {
        if (eventType == it->eventType && ((it->funcId == funcId) || (funcId == -1))) {
            SetEventListenerDeleting(it->isDeleting);
            soltIdSet.insert(it->slotId);
            it = listenerList_.erase(it);
        } else {
            ++it;
        }
    }
}

bool ObserverEventHandler::CheckEventTypeExist(int32_t slotId, TelephonyUpdateEventType eventType)
{
    for (auto &listen : listenerList_) {
        if (slotId == listen.slotId && eventType == listen.eventType) {
            return true;
        }
    }
    return false;
}

void ObserverEventHandler::CheckRemoveStateObserver(TelephonyUpdateEventType eventType, int32_t slotId, int32_t &result)
{
    if (!CheckEventTypeExist(slotId, eventType)) {
        int32_t removeRet = TelephonyStateManager::RemoveStateObserver(slotId, static_cast<uint32_t>(eventType));
        if (removeRet != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("ObserverEventHandler::RemoveStateObserver slotId %{public}d, eventType %{public}d fail!",
                slotId, static_cast<int32_t>(eventType));
            result = removeRet;
        }
    }
}

int32_t ObserverEventHandler::UnregisterEventListener(
    const TelephonyUpdateEventType eventType, int64_t funcId)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    if (listenerList_.empty()) {
        TELEPHONY_LOGI("UnregisterEventListener listener list is empty.");
        return TELEPHONY_SUCCESS;
    }

    std::set<int32_t> soltIdSet;
    RemoveEventListenerRegister(eventType, funcId, soltIdSet);
    int32_t result = TELEPHONY_SUCCESS;
    for (int32_t slotId : soltIdSet) {
        CheckRemoveStateObserver(eventType, slotId, result);
    }
    TELEPHONY_LOGI("ObserverEventHandler::UnregisterEventListener listenerList_ size=%{public}d",
        static_cast<int32_t>(listenerList_.size()));
    return result;
}

template<typename D, TelephonyUpdateEventType eventType>
void ObserverEventHandler::HandleCallbackInfoUpdate(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }

    std::unique_ptr<D> info = event->GetUniqueObject<D>();
    if (info == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }

    std::vector<EventListener> snapshot;
    {
        std::unique_lock<std::mutex> lock(operatorMutex_);
        for (const EventListener &listen : listenerList_) {
            if ((listen.eventType == eventType) && (listen.slotId == info->slotId_)) {
                snapshot.push_back(listen);
            }
        }
    }

    for (const EventListener &listen : snapshot) {
        D* data = new (std::nothrow) D(*info);
        if (data == nullptr) {
            TELEPHONY_LOGE("make data failed");
            continue;
        }
        WorkUpdated(listen, data);
        delete data;
    }
}

template<TelephonyUpdateEventType eventType>
void ObserverEventHandler::HandleCallbackVoidUpdate(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::vector<EventListener> snapshot;
    {
        std::unique_lock<std::mutex> lock(operatorMutex_);
        for (const EventListener &listen : listenerList_) {
            if ((listen.eventType == eventType)) {
                snapshot.push_back(listen);
            }
        }
    }

    for (const EventListener &listen : snapshot) {
        WorkUpdated(listen, nullptr);
    }
}

void ObserverEventHandler::WorkUpdated(const EventListener &listener,
    void *data)
{
    TELEPHONY_LOGD("ObserverEventHandler::WorkUpdated eventType is %{public}d", listener.eventType);
    if (listener.isDeleting == nullptr || *(listener.isDeleting)) {
        TELEPHONY_LOGI("listener is deleting");
        return;
    }

    switch (listener.eventType) {
        case TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE:
            WorkCallStateUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE:
            WorkSignalUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE:
            WorkNetworkStateUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE:
            WorkSimStateUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE:
            WorkCellInfomationUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE:
            WorkCellularDataConnectStateUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE:
            WorkCellularDataFlowUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_CFU_INDICATOR_UPDATE:
            WorkCfuIndicatorUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE:
            WorkVoiceMailMsgIndicatorUpdated(listener, data);
            break;
        case TelephonyUpdateEventType::EVENT_ICC_ACCOUNT_CHANGE:
            WorkIccAccountUpdated(listener, data);
            break;
        default:
            TELEPHONY_LOGE("ObserverEventHandler::WorkUpdated Unkonw Telephony UpdateEventType");
            return;
    }
}

void ObserverEventHandler::WorkCallStateUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    CallStateUpdateInfo *callStateInfo = static_cast<CallStateUpdateInfo *>(data);
    std::string phoneNumber = ToUtf8(callStateInfo->phoneNumber_);
    CCallStateInfo callbackValue = {
        .state = WrapCallState(callStateInfo->callState_),
        .number = MallocCString(phoneNumber)
    };
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkSignalUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    SignalUpdateInfo *infoListUpdateInfo = static_cast<SignalUpdateInfo *>(data);
    size_t infoSize = infoListUpdateInfo->signalInfoList_.size();
    if (infoSize <= 0) {
        TELEPHONY_LOGE("signalInfoList_ size error");
        return;
    }
    CSignalInformation* head =
        reinterpret_cast<CSignalInformation *>(malloc(sizeof(CSignalInformation) * infoSize));
    if (head == nullptr) {
        TELEPHONY_LOGE("ObserverEventHandler::WorkSignalUpdated malloc CSignalInformation failed.");
        return;
    }
    if (memset_s(head, sizeof(CSignalInformation) * infoSize, 0x00, sizeof(CSignalInformation) * infoSize) != EOK) {
        TELEPHONY_LOGE("memset_s failed");
        free(head);
        return;
    }
    CArraySignalInformation signalInformations = { .head = nullptr, .size = 0 };
    size_t validCount = 0;
    for (size_t i = 0; i < infoSize; i++) {
        sptr<SignalInformation> infoItem = infoListUpdateInfo->signalInfoList_[i];
        if (infoItem == nullptr) {
            TELEPHONY_LOGE("infoItem is nullptr, index=%{public}zu", i);
            continue;
        }
        head[validCount].signalType = WrapNetworkType(infoItem->GetNetworkType());
        head[validCount].signalLevel = infoItem->GetSignalLevel();
        head[validCount].dBm = infoItem->GetSignalIntensity();
        validCount++;
    }
    signalInformations.size = static_cast<int64_t>(validCount);
    signalInformations.head = head;
    void* argv = &(signalInformations);
    listener.callbackRef(argv);
    free(head);
}

void ObserverEventHandler::WorkNetworkStateUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    NetworkStateUpdateInfo *networkStateUpdateInfo = static_cast<NetworkStateUpdateInfo *>(data);
    const sptr<NetworkState> &networkState = networkStateUpdateInfo->networkState_;
    if (networkState == nullptr) {
        TELEPHONY_LOGE("networkState is nullptr");
        return;
    }
    std::string longOperatorName = networkState->GetLongOperatorName();
    std::string shortOperatorName = networkState->GetShortOperatorName();
    std::string plmnNumeric = networkState->GetPlmnNumeric();
    bool isRoaming = networkState->IsRoaming();
    int32_t regStatus = static_cast<int32_t>(networkState->GetRegStatus());
    bool isEmergency = networkState->IsEmergency();
    int32_t cfgTech = static_cast<int32_t>(networkState->GetCfgTech());
    int32_t nsaState = static_cast<int32_t>(networkState->GetNrState());
    CNetworkState callbackValue = {
        .longOperatorName = MallocCString(longOperatorName),
        .shortOperatorName = MallocCString(shortOperatorName),
        .plmnNumeric = MallocCString(plmnNumeric),
        .isRoaming = isRoaming,
        .regState = WrapRegState(regStatus),
        .cfgTech = WrapRadioTech(cfgTech),
        .nsaState = nsaState,
        .isCaActive = false,
        .isEmergency = isEmergency
    };
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkSimStateUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    SimStateUpdateInfo *simStateUpdateInfo = static_cast<SimStateUpdateInfo *>(data);
    int32_t cardType = static_cast<int32_t>(simStateUpdateInfo->type_);
    int32_t simState = static_cast<int32_t>(simStateUpdateInfo->state_);
    int32_t lockReason = static_cast<int32_t>(simStateUpdateInfo->reason_);
    CSimStateData callbackValue = {
        .cardType = cardType,
        .state = simState,
        .reason = lockReason
    };
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkCellInfomationUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
}

void ObserverEventHandler::WorkCellularDataConnectStateUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    CellularDataConnectState *context = static_cast<CellularDataConnectState *>(data);
    CDataConnectionStateInfo callbackValue = {
        .state = context->dataState_,
        .network = context->networkType_
    };
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkCellularDataFlowUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    CellularDataFlowUpdate *dataFlowInfo = static_cast<CellularDataFlowUpdate *>(data);
    void* argv = &(dataFlowInfo->flowType_);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkCfuIndicatorUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
}

void ObserverEventHandler::WorkVoiceMailMsgIndicatorUpdated(const EventListener &listener,
    void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is null");
        return;
    }
}

void ObserverEventHandler::WorkIccAccountUpdated(const EventListener &listener,
    void *data)
{
    if (!listener.callbackRef) {
        TELEPHONY_LOGE("callbackRef is nullptr");
        return;
    }
    void* argv = nullptr;
    listener.callbackRef(argv);
}
}
}
