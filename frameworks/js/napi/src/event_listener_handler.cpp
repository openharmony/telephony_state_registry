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

#include "event_listener_handler.h"

#include <cinttypes>

#include "call_manager_inner_type.h"
#include "event_listener_manager.h"
#include "inner_event.h"
#include "napi_parameter_util.h"
#include "napi_radio_types.h"
#include "napi_sim_type.h"
#include "napi_state_registry.h"
#include "napi_telephony_observer.h"
#include "napi_util.h"
#include "singleton.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"
#include "telephony_state_manager.h"
#include "update_contexts.h"

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
        case (int32_t)Telephony::TelCallState::CALL_STATUS_ACTIVE:
        case (int32_t)Telephony::TelCallState::CALL_STATUS_HOLDING:
        case (int32_t)Telephony::TelCallState::CALL_STATUS_DIALING:
        case (int32_t)Telephony::TelCallState::CALL_STATUS_ALERTING:
        case (int32_t)Telephony::TelCallState::CALL_STATUS_DISCONNECTING:
            return static_cast<int32_t>(CallState::CALL_STATE_OFFHOOK);
        case (int32_t)Telephony::TelCallState::CALL_STATUS_WAITING:
        case (int32_t)Telephony::TelCallState::CALL_STATUS_INCOMING:
            return static_cast<int32_t>(CallState::CALL_STATE_RINGING);
        case (int32_t)Telephony::TelCallState::CALL_STATUS_DISCONNECTED:
        case (int32_t)Telephony::TelCallState::CALL_STATUS_IDLE:
            return static_cast<int32_t>(CallState::CALL_STATE_IDLE);
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

napi_status NapiReturnToJS(napi_env env, napi_ref callbackRef, napi_value callbackVal)
{
    napi_value callbackFunc = nullptr;
    napi_get_reference_value(env, callbackRef, &callbackFunc);
    napi_value callbackValues[] = {callbackVal};
    napi_value recv = nullptr;
    napi_get_undefined(env, &recv);
    napi_value callbackResult = nullptr;
    napi_status status =
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &callbackResult);
    if (status != napi_ok) {
        TELEPHONY_LOGE("NapiReturnToJS napi_call_function return error : %{public}d", status);
    }
    auto handler = DelayedSingleton<EventListenerHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return status;
    }
    handler->SetCallbackCompleteToListener(callbackRef);
    return status;
}

napi_value SignalInfoConversion(napi_env env, int32_t type, int32_t level)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "signalType", type);
    SetPropertyToNapiObject(env, val, "signalLevel", level);
    return val;
}

napi_value DataOfNetworkConversion(napi_env env, const GsmCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "lac", info.GetLac());
    SetPropertyToNapiObject(env, val, "cellId", info.GetCellId());
    SetPropertyToNapiObject(env, val, "arfcn", info.GetArfcn());
    SetPropertyToNapiObject(env, val, "basic", info.GetBsic());
    SetPropertyToNapiObject(env, val, "mcc", info.GetMcc());
    SetPropertyToNapiObject(env, val, "mnc", info.GetMnc());
    return val;
}

napi_value DataOfNetworkConversion(napi_env env, const LteCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "cgi", 0);
    SetPropertyToNapiObject(env, val, "pci", info.GetPci());
    SetPropertyToNapiObject(env, val, "tac", info.GetTac());
    SetPropertyToNapiObject(env, val, "earfcn", info.GetArfcn());
    SetPropertyToNapiObject(env, val, "bandwith", 0);
    SetPropertyToNapiObject(env, val, "mcc", info.GetMnc());
    SetPropertyToNapiObject(env, val, "mnc", info.GetMnc());
    SetPropertyToNapiObject(env, val, "isSupportEndc", false);
    return val;
}

napi_value DataOfNetworkConversion(napi_env env, const WcdmaCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "lac", info.GetLac());
    SetPropertyToNapiObject(env, val, "cellId", info.GetCellId());
    SetPropertyToNapiObject(env, val, "psc", info.GetPsc());
    SetPropertyToNapiObject(env, val, "uarfcn", info.GetArfcn());
    SetPropertyToNapiObject(env, val, "mcc", info.GetMcc());
    SetPropertyToNapiObject(env, val, "mnc", info.GetMnc());
    return val;
}

napi_value CellInfoConversion(napi_env env, const CellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    CellInformation::CellType networkType = info.GetNetworkType();
    SetPropertyToNapiObject(env, val, "networkType", static_cast<int32_t>(networkType));
    SetPropertyToNapiObject(env, val, "isCamped", info.GetIsCamped());
    SetPropertyToNapiObject(env, val, "timeStamp", static_cast<int64_t>(info.GetTimeStamp()));
    SetPropertyToNapiObject(env, val, "signalInfomation",
        SignalInfoConversion(env, static_cast<int32_t>(networkType), info.GetSignalLevel()));

    switch (networkType) {
        case CellInformation::CellType::CELL_TYPE_GSM:
            napi_set_named_property(
                env, val, "data", DataOfNetworkConversion(env, static_cast<const GsmCellInformation &>(info)));
            break;
        case CellInformation::CellType::CELL_TYPE_LTE:
            napi_set_named_property(
                env, val, "data", DataOfNetworkConversion(env, static_cast<const LteCellInformation &>(info)));
            break;
        case CellInformation::CellType::CELL_TYPE_WCDMA:
            napi_set_named_property(
                env, val, "data", DataOfNetworkConversion(env, static_cast<const WcdmaCellInformation &>(info)));
            break;
        default:
            break;
    }
    return val;
}

bool InitLoop(napi_env env, uv_loop_s **loop)
{
#if NAPI_VERSION >= 2
    napi_status status = napi_get_uv_event_loop(env, loop);
    if (status != napi_ok) {
        TELEPHONY_LOGE("napi_get_uv_event_loop napi_status = %{public}d", status);
        return false;
    }
#endif // NAPI_VERSION >= 2
    return *loop != nullptr;
}
} // namespace

EventListenerHandler::EventListenerHandler() : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create())
{
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<CallStateContext, CallStateUpdateInfo,
            TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE>;
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<SignalListContext, SignalUpdateInfo,
            TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE>;
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<NetworkStateContext, NetworkStateUpdateInfo,
            TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE>;
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<SimStateContext, SimStateUpdateInfo,
            TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE>;
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CELL_INFOMATION_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<CellInfomationContext, CellInfomationUpdate,
            TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE>;
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<CellularDataConnectStateContext, CellularDataConnectState,
            TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE>;
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_FLOW_UPDATE] =
        &EventListenerHandler::HandleCallbackInfoUpdate<CellularDataFlowContext, CellularDataFlowUpdate,
            TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE>;

    workFuncMap_[TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE] = &EventListenerHandler::WorkCallStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE] =
        &EventListenerHandler::WorkSignalUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE] =
        &EventListenerHandler::WorkNetworkStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE] = &EventListenerHandler::WorkSimStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE] =
        &EventListenerHandler::WorkCellInfomationUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE] =
        &EventListenerHandler::WorkCellularDataConnectStateUpdate;
    workFuncMap_[TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE] =
        &EventListenerHandler::WorkCellularDataFlowUpdate;
}

EventListenerHandler::~EventListenerHandler()
{
    handleFuncMap_.clear();
    workFuncMap_.clear();
    listenerList_.clear();
    registerStateMap_.clear();
}

void EventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("EventListenerHandler::ProcessEvent event is nullptr");
        return;
    }
    auto itor = handleFuncMap_.find(static_cast<TelephonyCallbackEventId>(event->GetInnerEventId()));
    if (itor != handleFuncMap_.end()) {
        (this->*(itor->second))(event);
    }
}

std::optional<int32_t> EventListenerHandler::RegisterEventListener(EventListener &eventListener)
{
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    eventListener.index = listenerList_.size();
    listenerList_.push_back(eventListener);
    bool registered = IsEventTypeRegistered(eventListener.slotId, eventListener.eventType);
    if (!registered) {
        NapiTelephonyObserver *telephonyObserver = std::make_unique<NapiTelephonyObserver>().release();
        if (telephonyObserver == nullptr) {
            TELEPHONY_LOGE("error by telephonyObserver nullptr");
            return std::make_optional<int32_t>(ERROR_DEFAULT);
        }
        sptr<TelephonyObserverBroker> observer(telephonyObserver);
        if (observer == nullptr) {
            TELEPHONY_LOGE("error by observer nullptr");
            return std::make_optional<int32_t>(ERROR_DEFAULT);
        }
        int32_t addResult = TelephonyStateManager::AddStateObserver(
            observer, eventListener.slotId, ToUint32t(eventListener.eventType), false);
        if (addResult == TELEPHONY_SUCCESS) {
            ManageRegistrants(eventListener.slotId, eventListener.eventType, true);
        } else {
            TELEPHONY_LOGE("AddStateObserver failed, ret=%{public}d!", addResult);
            return std::make_optional<int32_t>(addResult);
        }
    }
    return std::nullopt;
}

std::optional<int32_t> EventListenerHandler::UnregisterEventListener(
    int32_t slotId, TelephonyUpdateEventType eventType)
{
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    if (listenerList_.empty()) {
        TELEPHONY_LOGE("UnregisterEventListener listener list is empty.");
        return std::nullopt;
    }
    if (!IsEventTypeRegistered(slotId, eventType)) {
        TELEPHONY_LOGE(
            "UnregisterEventListener eventType %{public}d was not registered!", static_cast<int32_t>(eventType));
        return std::nullopt;
    }

    std::atomic_uint32_t allListenersCallbackComplete = 1;
    do {
        for (const EventListener &listener : listenerList_) {
            if (listener.eventType == eventType) {
                allListenersCallbackComplete &= listener.callbackComplete;
            }
        }
    } while (!allListenersCallbackComplete);

    ManageRegistrants(slotId, eventType, false);
    int32_t result = TelephonyStateManager::RemoveStateObserver(slotId, ToUint32t(eventType));
    return (result == TELEPHONY_SUCCESS) ? std::nullopt : std::make_optional<int32_t>(result);
}

void EventListenerHandler::RemoveListener(TelephonyUpdateEventType eventType)
{
    listenerList_.remove_if([eventType](EventListener listener) -> bool {
        bool matched = listener.eventType == eventType;
        if (matched) {
            if (listener.env != nullptr && listener.callbackRef != nullptr) {
                napi_delete_reference(listener.env, listener.callbackRef);
            }
        }
        return matched;
    });
}

void EventListenerHandler::SetCallbackCompleteToListener(napi_ref ref, bool flag)
{
    for (EventListener &listen : listenerList_) {
        if (listen.callbackRef == ref) {
            listen.callbackComplete = flag;
        }
    }
}

void EventListenerHandler::ManageRegistrants(
    uint32_t slotId, const TelephonyUpdateEventType eventType, bool isRegister)
{
    auto itor = registerStateMap_.find(slotId);
    if (itor != registerStateMap_.end()) {
        if (isRegister) {
            itor->second.insert(eventType);
        } else {
            itor->second.erase(eventType);
        }
    } else {
        std::set<TelephonyUpdateEventType> &&typeInfo {eventType};
        registerStateMap_.insert(std::make_pair(slotId, typeInfo));
    }
}

bool EventListenerHandler::IsEventTypeRegistered(uint32_t slotId, const TelephonyUpdateEventType eventType) const
{
    auto itor = registerStateMap_.find(slotId);
    return (itor != registerStateMap_.end() ? itor->second.count(eventType) : false);
}

template<typename T, typename D, TelephonyUpdateEventType eventType>
void EventListenerHandler::HandleCallbackInfoUpdate(const AppExecFwk::InnerEvent::Pointer &event)
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

    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    for (const EventListener &listen : listenerList_) {
        if ((listen.eventType == eventType) && (listen.slotId == info->slotId_)) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(listen.env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            T *context = std::make_unique<T>().release();
            if (context == nullptr) {
                TELEPHONY_LOGE("make context failed");
                break;
            }
            *(static_cast<EventListener *>(context)) = listen;
            *context = *info;
            context->callbackComplete = false;
            uv_work_t *work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = static_cast<void *>(context);
            uv_queue_work(
                loop, work, [](uv_work_t *) {}, (workFuncMap_.find(eventType))->second);
        }
    }
}

void EventListenerHandler::WorkCallStateUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CallStateContext> callStateInfo(static_cast<CallStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    napi_create_object(callStateInfo->env, &callbackValue);
    int32_t wrappedCallState = WrapCallState(callStateInfo->callState);
    std::string number = NapiUtil::ToUtf8(callStateInfo->phoneNumber);
    SetPropertyToNapiObject(callStateInfo->env, callbackValue, "state", wrappedCallState);
    SetPropertyToNapiObject(callStateInfo->env, callbackValue, "number", number);
    NapiReturnToJS(callStateInfo->env, callStateInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkSignalUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<SignalListContext> infoListUpdateInfo(static_cast<SignalListContext *>(work->data));
    napi_value callbackValue = nullptr;
    const napi_env &env = infoListUpdateInfo->env;
    napi_create_array(env, &callbackValue);
    size_t infoSize = infoListUpdateInfo->signalInfoList.size();
    for (size_t i = 0; i < infoSize; ++i) {
        sptr<SignalInformation> infoItem = infoListUpdateInfo->signalInfoList[i];
        napi_value info = nullptr;
        napi_create_object(env, &info);
        SetPropertyToNapiObject(env, info, "signalType", WrapNetworkType(infoItem->GetNetworkType()));
        SetPropertyToNapiObject(env, info, "signalLevel", infoItem->GetSignalLevel());
        napi_set_element(env, callbackValue, i, info);
    }
    NapiReturnToJS(env, infoListUpdateInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkNetworkStateUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<NetworkStateContext> networkStateUpdateInfo(static_cast<NetworkStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    const napi_env &env = networkStateUpdateInfo->env;
    const sptr<NetworkState> &networkState = networkStateUpdateInfo->networkState;
    napi_create_object(env, &callbackValue);
    std::string longOperatorName = networkState->GetLongOperatorName();
    std::string shortOperatorName = networkState->GetShortOperatorName();
    std::string plmnNumeric = networkState->GetPlmnNumeric();
    bool isRoaming = networkState->IsRoaming();
    int32_t regStatus = static_cast<int32_t>(networkState->GetRegStatus());
    bool isEmergency = networkState->IsEmergency();
    SetPropertyToNapiObject(env, callbackValue, "longOperatorName", longOperatorName);
    SetPropertyToNapiObject(env, callbackValue, "shortOperatorName", shortOperatorName);
    SetPropertyToNapiObject(env, callbackValue, "plmnNumeric", plmnNumeric);
    SetPropertyToNapiObject(env, callbackValue, "isRoaming", isRoaming);
    SetPropertyToNapiObject(env, callbackValue, "regStatus", WrapRegState(regStatus));
    SetPropertyToNapiObject(env, callbackValue, "isEmergency", isEmergency);
    NapiReturnToJS(env, networkStateUpdateInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkSimStateUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<SimStateContext> simStateUpdateInfo(static_cast<SimStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    int32_t cardType = static_cast<int32_t>(simStateUpdateInfo->cardType);
    int32_t simState = static_cast<int32_t>(simStateUpdateInfo->simState);
    int32_t lockReason = static_cast<int32_t>(simStateUpdateInfo->reason);
    napi_create_object(simStateUpdateInfo->env, &callbackValue);
    SetPropertyToNapiObject(simStateUpdateInfo->env, callbackValue, "type", cardType);
    SetPropertyToNapiObject(simStateUpdateInfo->env, callbackValue, "state", simState);
    SetPropertyToNapiObject(simStateUpdateInfo->env, callbackValue, "reason", lockReason);
    NapiReturnToJS(simStateUpdateInfo->env, simStateUpdateInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkCellInfomationUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CellInfomationContext> cellInfo(static_cast<CellInfomationContext *>(work->data));
    napi_value callbackValue = nullptr;
    napi_create_array(cellInfo->env, &callbackValue);
    for (size_t i = 0; i < cellInfo->cellInfoVec.size(); i++) {
        napi_value val = CellInfoConversion(cellInfo->env, *(cellInfo->cellInfoVec[i]));
        napi_set_element(cellInfo->env, callbackValue, i, val);
    }
    NapiReturnToJS(cellInfo->env, cellInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkCellularDataConnectStateUpdate(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CellularDataConnectStateContext> context(
        static_cast<CellularDataConnectStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    napi_create_object(context->env, &callbackValue);
    SetPropertyToNapiObject(context->env, callbackValue, "state", context->dataState);
    SetPropertyToNapiObject(context->env, callbackValue, "network", context->networkType);
    NapiReturnToJS(context->env, context->callbackRef, callbackValue);
}

void EventListenerHandler::WorkCellularDataFlowUpdate(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CellularDataFlowContext> dataFlowInfo(static_cast<CellularDataFlowContext *>(work->data));
    napi_value callbackValue = GetNapiValue(dataFlowInfo->env, dataFlowInfo->flowType_);
    NapiReturnToJS(dataFlowInfo->env, dataFlowInfo->callbackRef, callbackValue);
}
} // namespace Telephony
} // namespace OHOS
