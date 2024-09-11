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

#include "napi_radio_types.h"
#include "observer_event_handler.h"
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
    if (event == nullptr) {
        TELEPHONY_LOGE("EventListenerHandler::ProcessEvent event is nullptr");
        return;
    }
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

    std::unique_lock<std::mutex> lock(operatorMutex_);
    for (const EventListener &listen : listenerList_) {
        if ((listen.eventType == eventType) && (listen.slotId == info->slotId_)) {
            uv_work_t *work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            std::unique_ptr<D> context = std::move(info);
            D* data = context.release();
             if (data == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = static_cast<void *>(data);
            WorkUpdated(listen, work, lock);
            delete work;
            work = nullptr;
        }
    }
}

template<TelephonyUpdateEventType eventType>
void ObserverEventHandler::HandleCallbackVoidUpdate(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_lock<std::mutex> lock(operatorMutex_);
    for (const EventListener &listen : listenerList_) {
        if ((listen.eventType == eventType)) {
            uv_work_t *work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            EventListener *listener = new EventListener();
            listener->eventType = listen.eventType;
            listener->slotId = listen.slotId;
            listener->funcId = listen.funcId;
            listener->callbackRef = listen.callbackRef;
            listener->isDeleting = listen.isDeleting;
            work->data = static_cast<void *>(listener);
            WorkUpdated(listen, work, lock);
            delete listener;
            delete work;
        }
    }
}

void ObserverEventHandler::WorkUpdated(const EventListener &listener, uv_work_t *work)
{
    TELEPHONY_LOGD("ObserverEventHandler::WorkUpdated eventType is %{public}d", *(listener.eventType));
    if (listener.isDeleting == nullptr || *(listener.isDeleting)) {
        TELEPHONY_LOGI("listener is deleting");
        return;
    }

    switch (listener.eventType) {
        case TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE:
            WorkCallStateUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE:
            WorkSignalUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE:
            WorkNetworkStateUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE:
            WorkSimStateUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE:
            WorkCellInfomationUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE:
            WorkCellularDataConnectStateUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE:
            WorkCellularDataFlowUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_CFU_INDICATOR_UPDATE:
            WorkCfuIndicatorUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE:
            WorkVoiceMailMsgIndicatorUpdated(listener, work, lock);
            break;
        case TelephonyUpdateEventType::EVENT_ICC_ACCOUNT_CHANGE:
            WorkIccAccountUpdated(listener, work, lock);
            break;
        default:
          TELEPHONY_LOGE("ObserverEventHandler::WorkUpdated Unkonw Telephony UpdateEventType");
          return;
    }
}

void ObserverEventHandler::WorkCallStateUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CallStateUpdateInfo> callStateInfo(static_cast<CallStateUpdateInfo *>(work->data));
    std::string phoneNumber = ToUtf8(callStateInfo->phoneNumber_);
    CCallStateInfo callbackValue = {
        .state = WrapCallState(callStateInfo->callState_),
        .number = MallocCString(phoneNumber)
    };
    lock.unlock();
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkSignalUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<SignalUpdateInfo> infoListUpdateInfo(static_cast<SignalUpdateInfo *>(work->data));
    size_t infoSize = infoListUpdateInfo->signalInfoList_.size();
    CSignalInformation* head =
        reinterpret_cast<CSignalInformation *>(malloc(sizeof(CSignalInformation) * infoSize));
    if (head == nullptr) {
        TELEPHONY_LOGE("ObserverEventHandler::WorkSignalUpdated malloc CSignalInformation failed.");
        return;
    }
    CArraySignalInformation signalInformations = { .head = nullptr, .size = 0 };
    for (size_t i = 0; i < infoSize; i++) {
        sptr<SignalInformation> infoItem = infoListUpdateInfo->signalInfoList_[i];
        head[i].signalType = WrapNetworkType(infoItem->GetNetworkType());
        head[i].signalLevel = infoItem->GetSignalLevel();
        head[i].dBm = infoItem->GetSignalIntensity();
    }
    signalInformations.size = static_cast<int64_t>(infoSize);
    signalInformations.head = head;
    lock.unlock();
    void* argv = &(signalInformations);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkNetworkStateUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<NetworkStateUpdateInfo> networkStateUpdateInfo(static_cast<NetworkStateUpdateInfo *>(work->data));
    const sptr<NetworkState> &networkState = networkStateUpdateInfo->networkState_;
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
    lock.unlock();
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkSimStateUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<SimStateUpdateInfo> simStateUpdateInfo(static_cast<SimStateUpdateInfo *>(work->data));
    int32_t cardType = static_cast<int32_t>(simStateUpdateInfo->type_);
    int32_t simState = static_cast<int32_t>(simStateUpdateInfo->state_);
    int32_t lockReason = static_cast<int32_t>(simStateUpdateInfo->reason_);
    CSimStateData callbackValue = {
        .cardType = cardType,
        .state = simState,
        .reason = lockReason
    };
    lock.unlock();
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkCellInfomationUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
}

void ObserverEventHandler::WorkCellularDataConnectStateUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CellularDataConnectState> context(
        static_cast<CellularDataConnectState *>(work->data));
    CDataConnectionStateInfo callbackValue = {
        .state = context->dataState_,
        .network = context->networkType_
    };
    lock.unlock();
    void* argv = &(callbackValue);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkCellularDataFlowUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CellularDataFlowUpdate> dataFlowInfo(static_cast<CellularDataFlowUpdate *>(work->data));
    lock.unlock();
    void* argv = &(dataFlowInfo->flowType_);
    listener.callbackRef(argv);
}

void ObserverEventHandler::WorkCfuIndicatorUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
}

void ObserverEventHandler::WorkVoiceMailMsgIndicatorUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
}

void ObserverEventHandler::WorkIccAccountUpdated(const EventListener &listener,
    uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    lock.unlock();
    void* argv = nullptr;
    listener.callbackRef(argv);
}
}
}
