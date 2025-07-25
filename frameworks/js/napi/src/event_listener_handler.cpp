/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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

#include "event_listener_manager.h"
#include "inner_event.h"
#include "napi_parameter_util.h"
#include "napi_radio_types.h"
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

napi_status NapiReturnToJS(
    napi_env env, napi_ref callbackRef, napi_value callbackVal, std::unique_lock<std::mutex> &lock)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
        napi_close_handle_scope(env, scope);
        return napi_ok;
    }
    if (callbackRef == nullptr) {
        TELEPHONY_LOGE("NapiReturnToJS callbackRef is nullptr");
        napi_close_handle_scope(env, scope);
        return napi_ok;
    }
    napi_value callbackFunc = nullptr;
    napi_get_reference_value(env, callbackRef, &callbackFunc);
    lock.unlock();
    napi_value callbackValues[] = { callbackVal };
    napi_value recv = nullptr;
    napi_get_undefined(env, &recv);
    napi_value callbackResult = nullptr;
    napi_status status =
        napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &callbackResult);
    if (status != napi_ok) {
        TELEPHONY_LOGE("NapiReturnToJS napi_call_function return error : %{public}d", status);
    }
    napi_close_handle_scope(env, scope);
    return status;
}

napi_value SignalInfoConversion(napi_env env, int32_t type, int32_t level, int32_t signalIntensity)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "signalType", type);
    SetPropertyToNapiObject(env, val, "signalLevel", level);
    SetPropertyToNapiObject(env, val, "dBm", signalIntensity);
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
    SetPropertyToNapiObject(env, val, "cgi", info.GetCellId());
    SetPropertyToNapiObject(env, val, "pci", info.GetPci());
    SetPropertyToNapiObject(env, val, "tac", info.GetTac());
    SetPropertyToNapiObject(env, val, "earfcn", info.GetArfcn());
    SetPropertyToNapiObject(env, val, "bandwith", 0);
    SetPropertyToNapiObject(env, val, "mcc", info.GetMcc());
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

napi_value DataOfNetworkConversion(napi_env env, const NrCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    SetPropertyToNapiObject(env, val, "nrArfcn", info.GetArfcn());
    SetPropertyToNapiObject(env, val, "pci", info.GetPci());
    SetPropertyToNapiObject(env, val, "tac", info.GetTac());
    SetPropertyToNapiObject(env, val, "nci", info.GetNci());
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
    SetPropertyToNapiObject(env, val, "signalInformation",
        SignalInfoConversion(env, static_cast<int32_t>(networkType), info.GetSignalLevel(), info.GetSignalIntensity()));

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
        case CellInformation::CellType::CELL_TYPE_NR:
            napi_set_named_property(
                env, val, "data", DataOfNetworkConversion(env, static_cast<const NrCellInformation &>(info)));
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

std::map<TelephonyUpdateEventType,
    void (*)(uv_work_t *work, std::unique_lock<std::mutex> &lock)> EventListenerHandler::workFuncMap_;
std::mutex EventListenerHandler::operatorMutex_;

EventListenerHandler::EventListenerHandler() : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create())
{
    AddBasicHandlerToMap();
    AddNetworkHandlerToMap();
    AddWorkFuncToMap();
}

EventListenerHandler::~EventListenerHandler()
{
    handleFuncMap_.clear();
    workFuncMap_.clear();
    listenerList_.clear();
}

void EventListenerHandler::AddBasicHandlerToMap()
{
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<CallStateContext, CallStateUpdateInfo,
                TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<SimStateContext, SimStateUpdateInfo,
                TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<CellularDataConnectStateContext, CellularDataConnectState,
                TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_FLOW_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<CellularDataFlowContext, CellularDataFlowUpdate,
                TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CFU_INDICATOR_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<CfuIndicatorContext, CfuIndicatorUpdate,
                TelephonyUpdateEventType::EVENT_CFU_INDICATOR_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_VOICE_MAIL_MSG_INDICATOR_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<VoiceMailMsgIndicatorContext, VoiceMailMsgIndicatorUpdate,
                TelephonyUpdateEventType::EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_ICC_ACCOUNT_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackVoidUpdate<TelephonyUpdateEventType::EVENT_ICC_ACCOUNT_CHANGE>(event);
        };
}

void EventListenerHandler::AddNetworkHandlerToMap()
{
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<SignalListContext, SignalUpdateInfo,
                TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<NetworkStateContext, NetworkStateUpdateInfo,
                TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE>(event);
        };
    handleFuncMap_[TelephonyCallbackEventId::EVENT_ON_CELL_INFOMATION_UPDATE] =
        [this](const AppExecFwk::InnerEvent::Pointer &event) {
            HandleCallbackInfoUpdate<CellInfomationContext, CellInfomationUpdate,
                TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE>(event);
        };
}

void EventListenerHandler::AddWorkFuncToMap()
{
    workFuncMap_[TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE] = &EventListenerHandler::WorkCallStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE] = &EventListenerHandler::WorkSignalUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE] = &EventListenerHandler::WorkNetworkStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE] = &EventListenerHandler::WorkSimStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE] = &EventListenerHandler::WorkCellInfomationUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE] =
        &EventListenerHandler::WorkCellularDataConnectStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE] =
        &EventListenerHandler::WorkCellularDataFlowUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_CFU_INDICATOR_UPDATE] = &EventListenerHandler::WorkCfuIndicatorUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE] =
        &EventListenerHandler::WorkVoiceMailMsgIndicatorUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_ICC_ACCOUNT_CHANGE] = &EventListenerHandler::WorkIccAccountUpdated;
}

void EventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("EventListenerHandler::ProcessEvent event is nullptr");
        return;
    }
    auto eventId = static_cast<TelephonyCallbackEventId>(event->GetInnerEventId());
    if (eventId == TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE) {
        TELEPHONY_LOGD("process event %{public}d", eventId);
    } else {
        TELEPHONY_LOGI("process event %{public}d", eventId);
    }
    auto itor = handleFuncMap_.find(eventId);
    if (itor != handleFuncMap_.end()) {
        itor->second(event);
    }
}

int32_t EventListenerHandler::CheckEventListenerRegister(EventListener &eventListener)
{
    int32_t flag = EVENT_LISTENER_DIFF;
    for (auto &listen : listenerList_) {
        if (eventListener.env == listen.env && eventListener.slotId == listen.slotId &&
            eventListener.eventType == listen.eventType &&
            IsCallBackRegister(eventListener.env, eventListener.callbackRef, listen.callbackRef)) {
            flag = EVENT_LISTENER_SAME;
            return flag;
        }
        if (eventListener.slotId == listen.slotId && eventListener.eventType == listen.eventType) {
            flag = EVENT_LISTENER_SLOTID_AND_EVENTTYPE_SAME;
        }
    }
    return flag;
}

int32_t EventListenerHandler::RegisterEventListener(EventListener &eventListener)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    int32_t registerStatus = CheckEventListenerRegister(eventListener);
    if (registerStatus == EVENT_LISTENER_SAME) {
        return TELEPHONY_ERR_CALLBACK_ALREADY_REGISTERED;
    }
    if (registerStatus != EVENT_LISTENER_SLOTID_AND_EVENTTYPE_SAME) {
        NapiTelephonyObserver *telephonyObserver = std::make_unique<NapiTelephonyObserver>().release();
        if (telephonyObserver == nullptr) {
            TELEPHONY_LOGE("error by telephonyObserver nullptr");
            return TELEPHONY_ERR_LOCAL_PTR_NULL;
        }
        sptr<TelephonyObserverBroker> observer(telephonyObserver);
        if (observer == nullptr) {
            TELEPHONY_LOGE("error by observer nullptr");
            return TELEPHONY_ERR_LOCAL_PTR_NULL;
        }
        bool isUpdate = (eventListener.eventType == TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE ||
                        eventListener.eventType == TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE);
        int32_t addResult = TelephonyStateManager::AddStateObserver(
            observer, eventListener.slotId, ToUint32t(eventListener.eventType), isUpdate);
        if (addResult != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("AddStateObserver failed, ret=%{public}d!", addResult);
            return addResult;
        }
    }
    listenerList_.push_back(eventListener);
    TELEPHONY_LOGI("EventListenerHandler::RegisterEventListener listenerList_ size=%{public}d",
        static_cast<int32_t>(listenerList_.size()));
    return TELEPHONY_SUCCESS;
}

void EventListenerHandler::SetEventListenerDeleting(std::shared_ptr<bool> isDeleting)
{
    if (isDeleting == nullptr) {
        TELEPHONY_LOGE("isDeleting is nullptr");
        return;
    }
    *isDeleting = true;
}

bool EventListenerHandler::CheckEventTypeExist(int32_t slotId, TelephonyUpdateEventType eventType)
{
    for (auto &listen : listenerList_) {
        if (slotId == listen.slotId && eventType == listen.eventType) {
            return true;
        }
    }
    return false;
}

void EventListenerHandler::RemoveEventListenerRegister(napi_env env, TelephonyUpdateEventType eventType, napi_ref ref,
    std::list<EventListener> &removeListenerList, std::set<int32_t> &soltIdSet)
{
    std::list<EventListener>::iterator it = listenerList_.begin();
    while (it != listenerList_.end()) {
        if (env == it->env && eventType == it->eventType && IsCallBackRegister(env, ref, it->callbackRef)) {
            SetEventListenerDeleting(it->isDeleting);
            soltIdSet.insert(it->slotId);
            removeListenerList.push_back(*it);
            it = listenerList_.erase(it);
        } else {
            ++it;
        }
    }
}

void EventListenerHandler::RemoveEventListenerRegister(napi_env env, TelephonyUpdateEventType eventType,
    std::list<EventListener> &removeListenerList, std::set<int32_t> &soltIdSet)
{
    std::list<EventListener>::iterator it = listenerList_.begin();
    while (it != listenerList_.end()) {
        if (env == it->env && eventType == it->eventType) {
            SetEventListenerDeleting(it->isDeleting);
            soltIdSet.insert(it->slotId);
            removeListenerList.push_back(*it);
            it = listenerList_.erase(it);
        } else {
            ++it;
        }
    }
}

void EventListenerHandler::CheckRemoveStateObserver(TelephonyUpdateEventType eventType, int32_t slotId, int32_t &result)
{
    if (!CheckEventTypeExist(slotId, eventType)) {
        int32_t removeRet = TelephonyStateManager::RemoveStateObserver(slotId, ToUint32t(eventType));
        if (removeRet != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("EventListenerHandler::RemoveStateObserver slotId %{public}d, eventType %{public}d fail!",
                slotId, static_cast<int32_t>(eventType));
            result = removeRet;
        }
    }
}

int32_t EventListenerHandler::UnregisterEventListener(
    napi_env env, TelephonyUpdateEventType eventType, napi_ref ref, std::list<EventListener> &removeListenerList)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    if (listenerList_.empty()) {
        TELEPHONY_LOGI("UnregisterEventListener listener list is empty.");
        return TELEPHONY_SUCCESS;
    }

    std::set<int32_t> soltIdSet;
    RemoveEventListenerRegister(env, eventType, ref, removeListenerList, soltIdSet);
    int32_t result = TELEPHONY_SUCCESS;
    for (int32_t slotId : soltIdSet) {
        CheckRemoveStateObserver(eventType, slotId, result);
    }
    TELEPHONY_LOGI("EventListenerHandler::UnregisterEventListener listenerList_ size=%{public}d",
        static_cast<int32_t>(listenerList_.size()));
    return result;
}

int32_t EventListenerHandler::UnregisterEventListener(
    napi_env env, TelephonyUpdateEventType eventType, std::list<EventListener> &removeListenerList)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    if (listenerList_.empty()) {
        TELEPHONY_LOGI("UnregisterEventListener listener list is empty.");
        return TELEPHONY_SUCCESS;
    }

    std::set<int32_t> soltIdSet;
    RemoveEventListenerRegister(env, eventType, removeListenerList, soltIdSet);
    int32_t result = TELEPHONY_SUCCESS;
    for (int32_t slotId : soltIdSet) {
        CheckRemoveStateObserver(eventType, slotId, result);
    }
    TELEPHONY_LOGI("EventListenerHandler::UnregisterEventListener listenerList_ size=%{public}d",
        static_cast<int32_t>(listenerList_.size()));
    return result;
}

void EventListenerHandler::UnRegisterAllListener(napi_env env)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    if (listenerList_.empty()) {
        TELEPHONY_LOGI("UnRegisterAllListener listener list is empty.");
        return;
    } else {
        TELEPHONY_LOGI("UnRegisterAllListener listener list size start: %{public}d",
            static_cast<int32_t>(listenerList_.size()));
    }
    std::map<int32_t, std::set<TelephonyUpdateEventType>> removeTypeMap;
    listenerList_.remove_if([&](EventListener listener) -> bool {
        bool matched = listener.env == env;
        if (matched) {
            SetEventListenerDeleting(listener.isDeleting);
            if (!removeTypeMap.count(listener.slotId)) {
                std::set<TelephonyUpdateEventType> eventTypeSet;
                eventTypeSet.insert(listener.eventType);
                removeTypeMap.insert(std::make_pair(listener.slotId, eventTypeSet));
            } else {
                removeTypeMap[listener.slotId].insert(listener.eventType);
            }
            if (listener.env != nullptr && listener.callbackRef != nullptr) {
                napi_delete_reference(listener.env, listener.callbackRef);
                listener.callbackRef = nullptr;
            }
        }

        return matched;
    });

    int32_t result = TELEPHONY_SUCCESS;
    for (auto &elem : removeTypeMap) {
        for (auto &innerElem : elem.second) {
            CheckRemoveStateObserver(innerElem, elem.first, result);
        }
    }
    TELEPHONY_LOGI(
        "UnRegisterAllListener listener list size finish: %{public}d", static_cast<int32_t>(listenerList_.size()));
}

bool EventListenerHandler::IsCallBackRegister(napi_env env, napi_ref ref, napi_ref registeredRef) const
{
    napi_value callback = nullptr;
    napi_get_reference_value(env, ref, &callback);
    napi_value existCallBack = nullptr;
    napi_get_reference_value(env, registeredRef, &existCallBack);
    bool result = false;
    napi_strict_equals(env, callback, existCallBack, &result);
    return result;
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

    std::unique_lock<std::mutex> lock(operatorMutex_);
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
            uv_work_t *work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = static_cast<void *>(context);
            int32_t resultCode =
                uv_queue_work_with_qos(loop, work, [](uv_work_t *) {}, WorkUpdated, uv_qos_default);
            if (resultCode != 0) {
                delete context;
                context = nullptr;
                TELEPHONY_LOGE("HandleCallbackInfoUpdate failed, result: %{public}d", resultCode);
                delete work;
                work = nullptr;
                return;
            }
        }
    }
}

template<TelephonyUpdateEventType eventType>
void EventListenerHandler::HandleCallbackVoidUpdate(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }

    std::unique_lock<std::mutex> lock(operatorMutex_);
    for (const EventListener &listen : listenerList_) {
        if ((listen.eventType == eventType)) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(listen.env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            uv_work_t *work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            EventListener *listener = new EventListener();
            listener->env = listen.env;
            listener->eventType = listen.eventType;
            listener->slotId = listen.slotId;
            listener->callbackRef = listen.callbackRef;
            listener->isDeleting = listen.isDeleting;
            work->data = static_cast<void *>(listener);
            int32_t retVal =
                uv_queue_work_with_qos(loop, work, [](uv_work_t *) {}, WorkUpdated, uv_qos_default);
            if (retVal != 0) {
                delete listener;
                listener = nullptr;
                TELEPHONY_LOGE("HandleCallbackVoidUpdate failed, result: %{public}d", retVal);
                delete work;
                work = nullptr;
                return;
            }
        }
    }
}

void EventListenerHandler::WorkUpdated(uv_work_t *work, int status)
{
    std::unique_lock<std::mutex> lock(operatorMutex_);
    EventListener *listener = static_cast<EventListener *>(work->data);
    if (listener->eventType == TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE) {
        TELEPHONY_LOGD("WorkUpdated eventType is %{public}d", listener->eventType);
    } else {
        TELEPHONY_LOGI("WorkUpdated eventType is %{public}d", listener->eventType);
    }
    if (listener->isDeleting == nullptr || *(listener->isDeleting)) {
        TELEPHONY_LOGI("listener is deleting");
        delete listener;
        listener = nullptr;
        return;
    }
    if (workFuncMap_.find(listener->eventType) == workFuncMap_.end() ||
        workFuncMap_.find(listener->eventType)->second == nullptr) {
        TELEPHONY_LOGE("listener state update is nullptr");
        delete listener;
        listener = nullptr;
        return;
    }
    workFuncMap_.find(listener->eventType)->second(work, lock);
}

void EventListenerHandler::WorkCallStateUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    std::unique_ptr<CallStateContext> callStateInfo(static_cast<CallStateContext *>(work->data));
    const napi_env &env = callStateInfo->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue = nullptr;
    napi_create_object(callStateInfo->env, &callbackValue);
    int32_t wrappedCallState = WrapCallState(callStateInfo->callState);
    std::string number = NapiUtil::ToUtf8(callStateInfo->phoneNumber);
    SetPropertyToNapiObject(callStateInfo->env, callbackValue, "state", wrappedCallState);
    SetPropertyToNapiObject(callStateInfo->env, callbackValue, "number", number);
    NapiReturnToJS(callStateInfo->env, callStateInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkSignalUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    napi_handle_scope scope = nullptr;
    std::unique_ptr<SignalListContext> infoListUpdateInfo(static_cast<SignalListContext *>(work->data));
    napi_value callbackValue = nullptr;
    const napi_env &env = infoListUpdateInfo->env;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_create_array(env, &callbackValue);
    size_t infoSize = infoListUpdateInfo->signalInfoList.size();
    for (size_t i = 0; i < infoSize; ++i) {
        sptr<SignalInformation> infoItem = infoListUpdateInfo->signalInfoList[i];
        napi_value info = nullptr;
        napi_create_object(env, &info);
        SetPropertyToNapiObject(env, info, "signalType", WrapNetworkType(infoItem->GetNetworkType()));
        SetPropertyToNapiObject(env, info, "signalLevel", infoItem->GetSignalLevel());
        SetPropertyToNapiObject(env, info, "dBm", infoItem->GetSignalIntensity());
        napi_set_element(env, callbackValue, i, info);
    }
    NapiReturnToJS(env, infoListUpdateInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkNetworkStateUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    napi_handle_scope scope = nullptr;
    std::unique_ptr<NetworkStateContext> networkStateUpdateInfo(static_cast<NetworkStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    const napi_env &env = networkStateUpdateInfo->env;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    const sptr<NetworkState> &networkState = networkStateUpdateInfo->networkState;
    napi_create_object(env, &callbackValue);
    std::string longOperatorName = networkState->GetLongOperatorName();
    std::string shortOperatorName = networkState->GetShortOperatorName();
    std::string plmnNumeric = networkState->GetPlmnNumeric();
    bool isRoaming = networkState->IsRoaming();
    int32_t regStatus = static_cast<int32_t>(networkState->GetRegStatus());
    bool isEmergency = networkState->IsEmergency();
    int32_t cfgTech = static_cast<int32_t>(networkState->GetCfgTech());
    int32_t nsaState = static_cast<int32_t>(networkState->GetNrState());
    SetPropertyToNapiObject(env, callbackValue, "longOperatorName", longOperatorName);
    SetPropertyToNapiObject(env, callbackValue, "shortOperatorName", shortOperatorName);
    SetPropertyToNapiObject(env, callbackValue, "plmnNumeric", plmnNumeric);
    SetPropertyToNapiObject(env, callbackValue, "isRoaming", isRoaming);
    SetPropertyToNapiObject(env, callbackValue, "regState", WrapRegState(regStatus));
    SetPropertyToNapiObject(env, callbackValue, "isEmergency", isEmergency);
    SetPropertyToNapiObject(env, callbackValue, "cfgTech", WrapRadioTech(cfgTech));
    SetPropertyToNapiObject(env, callbackValue, "nsaState", nsaState);
    SetPropertyToNapiObject(env, callbackValue, "isCaActive", false);
    NapiReturnToJS(env, networkStateUpdateInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkSimStateUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    napi_handle_scope scope = nullptr;
    std::unique_ptr<SimStateContext> simStateUpdateInfo(static_cast<SimStateContext *>(work->data));
    const napi_env &env = simStateUpdateInfo->env;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue = nullptr;
    int32_t cardType = static_cast<int32_t>(simStateUpdateInfo->cardType);
    int32_t simState = static_cast<int32_t>(simStateUpdateInfo->simState);
    int32_t lockReason = static_cast<int32_t>(simStateUpdateInfo->reason);
    napi_create_object(simStateUpdateInfo->env, &callbackValue);
    SetPropertyToNapiObject(simStateUpdateInfo->env, callbackValue, "type", cardType);
    SetPropertyToNapiObject(simStateUpdateInfo->env, callbackValue, "state", simState);
    SetPropertyToNapiObject(simStateUpdateInfo->env, callbackValue, "reason", lockReason);
    NapiReturnToJS(simStateUpdateInfo->env, simStateUpdateInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkCellInfomationUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    std::unique_ptr<CellInfomationContext> cellInfo(static_cast<CellInfomationContext *>(work->data));
    napi_value callbackValue = nullptr;
    const napi_env &env = cellInfo->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_create_array(cellInfo->env, &callbackValue);
    for (size_t i = 0; i < cellInfo->cellInfoVec.size(); i++) {
        napi_value val = CellInfoConversion(cellInfo->env, *(cellInfo->cellInfoVec[i]));
        napi_set_element(cellInfo->env, callbackValue, i, val);
    }
    NapiReturnToJS(cellInfo->env, cellInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkCellularDataConnectStateUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    napi_handle_scope scope = nullptr;
    std::unique_ptr<CellularDataConnectStateContext> context(
        static_cast<CellularDataConnectStateContext *>(work->data));
    const napi_env &env = context->env;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue = nullptr;
    napi_create_object(context->env, &callbackValue);
    SetPropertyToNapiObject(context->env, callbackValue, "state", context->dataState);
    SetPropertyToNapiObject(context->env, callbackValue, "network", context->networkType);
    NapiReturnToJS(context->env, context->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkCellularDataFlowUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    std::unique_ptr<CellularDataFlowContext> dataFlowInfo(static_cast<CellularDataFlowContext *>(work->data));
    const napi_env &env = dataFlowInfo->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue = GetNapiValue(dataFlowInfo->env, dataFlowInfo->flowType_);
    NapiReturnToJS(dataFlowInfo->env, dataFlowInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkCfuIndicatorUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<CfuIndicatorContext> cfuIndicatorInfo(static_cast<CfuIndicatorContext *>(work->data));
    const napi_env &env = cfuIndicatorInfo->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue = GetNapiValue(cfuIndicatorInfo->env, cfuIndicatorInfo->cfuResult_);
    NapiReturnToJS(cfuIndicatorInfo->env, cfuIndicatorInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkVoiceMailMsgIndicatorUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<VoiceMailMsgIndicatorContext> voiceMailMsgIndicatorInfo(
        static_cast<VoiceMailMsgIndicatorContext *>(work->data));
    const napi_env &env = voiceMailMsgIndicatorInfo->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue =
        GetNapiValue(voiceMailMsgIndicatorInfo->env, voiceMailMsgIndicatorInfo->voiceMailMsgResult_);
    NapiReturnToJS(voiceMailMsgIndicatorInfo->env, voiceMailMsgIndicatorInfo->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}

void EventListenerHandler::WorkIccAccountUpdated(uv_work_t *work, std::unique_lock<std::mutex> &lock)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<EventListener> UpdateIccAccount(static_cast<EventListener *>(work->data));
    const napi_env &env = UpdateIccAccount->env;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    if (scope == nullptr) {
        TELEPHONY_LOGE("scope is nullptr");
    }
    napi_value callbackValue = nullptr;
    napi_create_object(UpdateIccAccount->env, &callbackValue);
    NapiReturnToJS(UpdateIccAccount->env, UpdateIccAccount->callbackRef, callbackValue, lock);
    napi_close_handle_scope(env, scope);
}
} // namespace Telephony
} // namespace OHOS
