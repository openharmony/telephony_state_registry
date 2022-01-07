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

#include "ability.h"
#include "inner_event.h"
#include "singleton.h"

#include "call_manager_inner_type.h"
#include "event_listener_manager.h"
#include "napi_radio_types.h"
#include "napi_state_registry.h"
#include "napi_telephony_observer.h"
#include "napi_util.h"
#include "telephony_errors.h"
#include "telephony_state_manager.h"
#include "update_contexts.h"

namespace OHOS {
namespace Telephony {
namespace {
std::map<TelephonyUpdateEventType, bool> registerStateMap {
    {TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE, false},
    {TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE, false},
    {TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE, false},
    {TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE, false},
    {TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE, false},
};

void SetRegisterState(const TelephonyUpdateEventType eventType, bool regist)
{
    auto itor = registerStateMap.find(eventType);
    if (itor != registerStateMap.end()) {
        registerStateMap[eventType] = regist;
    } else {
        TELEPHONY_LOGE("AddEventType mismatch eventType = %{public}u", static_cast<uint32_t>(eventType));
    }
}

bool IsEventTypeRegistered(const TelephonyUpdateEventType eventType)
{
    auto itor = registerStateMap.find(eventType);
    return (itor != registerStateMap.end() ? itor->second : false);
}

std::u16string GetBundleName(napi_env env)
{
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        TELEPHONY_LOGE("can't get global instance for %{public}d", status);
        return u"";
    }

    napi_value abilityObj = nullptr;
    status = napi_get_named_property(env, global, "ability", &abilityObj);
    if (status != napi_ok || abilityObj == nullptr) {
        TELEPHONY_LOGE("can't get ability obj for %{public}d", status);
        return u"";
    }

    OHOS::AppExecFwk::Ability *ability = nullptr;
    status = napi_get_value_external(env, abilityObj, (void **)&ability);
    if (status != napi_ok || ability == nullptr) {
        TELEPHONY_LOGE("get ability from property failed for %{public}d", status);
        return u"";
    }

    return NapiUtil::ToUtf16(ability->GetBundleName());
}

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
        case Telephony::CALL_STATUS_ACTIVE:
        case Telephony::CALL_STATUS_HOLDING:
        case Telephony::CALL_STATUS_DIALING:
        case Telephony::CALL_STATUS_ALERTING:
        case Telephony::CALL_STATUS_DISCONNECTING:
            return static_cast<int32_t>(CallState::CALL_STATE_OFFHOOK);
        case Telephony::CALL_STATUS_WAITING:
        case Telephony::CALL_STATUS_INCOMING:
            return static_cast<int32_t>(CallState::CALL_STATE_RINGING);
        case Telephony::CALL_STATUS_DISCONNECTED:
        case Telephony::CALL_STATUS_IDLE:
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
    return napi_call_function(env, recv, callbackFunc, std::size(callbackValues), callbackValues, &callbackResult);
}

napi_value SignalInfoConversion(napi_env env, int32_t type, int32_t level)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    napi_set_named_property(env, val, "signalType", GetNapiValue(env, type));
    napi_set_named_property(env, val, "signalLevel", GetNapiValue(env, level));
    return val;
}

napi_value DataOfNetworkConversion(napi_env env, const GsmCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    napi_set_named_property(env, val, "lac", GetNapiValue(env, info.GetLac()));
    napi_set_named_property(env, val, "cellId", GetNapiValue(env, info.GetCellId()));
    napi_set_named_property(env, val, "arfcn", GetNapiValue(env, info.GetArfcn()));
    napi_set_named_property(env, val, "basic", GetNapiValue(env, info.GetBsic()));
    napi_set_named_property(env, val, "mcc", GetNapiValue(env, info.GetMcc()));
    napi_set_named_property(env, val, "mnc", GetNapiValue(env, info.GetMnc()));
    return val;
}

napi_value DataOfNetworkConversion(napi_env env, const LteCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    napi_set_named_property(env, val, "cgi", GetNapiValue(env, 0));
    napi_set_named_property(env, val, "pci", GetNapiValue(env, info.GetPci()));
    napi_set_named_property(env, val, "tac", GetNapiValue(env, info.GetTac()));
    napi_set_named_property(env, val, "earfcn", GetNapiValue(env, info.GetArfcn()));
    napi_set_named_property(env, val, "bandwith", GetNapiValue(env, 0));
    napi_set_named_property(env, val, "mcc", GetNapiValue(env, info.GetMnc()));
    napi_set_named_property(env, val, "mnc", GetNapiValue(env, info.GetMnc()));
    napi_set_named_property(env, val, "isSupportEndc", GetNapiValue(env, false));
    return val;
}

napi_value DataOfNetworkConversion(napi_env env, const WcdmaCellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    napi_set_named_property(env, val, "lac", GetNapiValue(env, info.GetLac()));
    napi_set_named_property(env, val, "cellId", GetNapiValue(env, info.GetCellId()));
    napi_set_named_property(env, val, "psc", GetNapiValue(env, info.GetPsc()));
    napi_set_named_property(env, val, "uarfcn", GetNapiValue(env, info.GetArfcn()));
    napi_set_named_property(env, val, "mcc", GetNapiValue(env, info.GetMcc()));
    napi_set_named_property(env, val, "mnc", GetNapiValue(env, info.GetMnc()));
    return val;
}

napi_value CellInfoConversion(napi_env env, const CellInformation &info)
{
    napi_value val = nullptr;
    napi_create_object(env, &val);
    CellInformation::CellType networkType = info.GetNetworkType();
    napi_set_named_property(env, val, "networkType", GetNapiValue(env, static_cast<int32_t>(networkType)));
    napi_set_named_property(env, val, "isCamped", GetNapiValue(env, info.GetIsCamped()));
    napi_set_named_property(env, val, "timeStamp", GetNapiValue(env, static_cast<int32_t>(info.GetTimeStamp())));
    napi_set_named_property(env, val, "signalInfomation",
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
} // namespace

EventListenerHandler::EventListenerHandler() : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create())
{
    InitProcessFunc();
}

void EventListenerHandler::InitProcessFunc()
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

    workFuncMap_[TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE] = &EventListenerHandler::WorkCallStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE] =
        &EventListenerHandler::WorkSignalUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE] =
        &EventListenerHandler::WorkNetworkStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE] = &EventListenerHandler::WorkSimStateUpdated;
    workFuncMap_[TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE] =
        &EventListenerHandler::WorkCellInfomationUpdated;
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

std::pair<bool, int32_t> EventListenerHandler::AddEventListener(EventListener &eventListener)
{
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    eventListener.index = listenerList_.size();
    listenerList_.push_back(std::move(eventListener));
    bool registered = IsEventTypeRegistered(eventListener.eventType);
    if (!registered) {
        auto telephonyObserver = std::make_unique<NapiTelephonyObserver>().release();
        if (telephonyObserver == nullptr) {
            TELEPHONY_LOGE("error by telephonyObserver nullptr");
            return std::make_pair(false, ERROR_DEFAULT);
        }
        sptr<TelephonyObserverBroker> observer(telephonyObserver);
        if (observer == nullptr) {
            TELEPHONY_LOGE("error by observer nullptr");
            return std::make_pair(false, ERROR_DEFAULT);
        }
        std::u16string packageName = GetBundleName(eventListener.env);
        int32_t addResult = TelephonyStateManager::AddStateObserver(
            observer, eventListener.slotId, ToUint32t(eventListener.eventType), packageName, false);
        bool addSuccess = (addResult == TELEPHONY_SUCCESS);
        if (addSuccess) {
            SetRegisterState(eventListener.eventType, true);
        }
        return std::make_pair(addSuccess, addResult);
    }
    return std::make_pair(true, ERROR_NONE);
}

std::pair<bool, int32_t> EventListenerHandler::RemoveEventListener(
    int32_t slotId, const TelephonyUpdateEventType eventType)
{
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    listenerList_.remove_if([eventType](EventListener listener) -> bool {
        auto matched = listener.eventType == eventType;
        if (matched) {
            if (listener.env != nullptr && listener.callbackRef != nullptr) {
                napi_delete_reference(listener.env, listener.callbackRef);
            }
        };
        return matched;
    });
    SetRegisterState(eventType, false);
    int32_t removeResult = TelephonyStateManager::RemoveStateObserver(slotId, ToUint32t(eventType));
    bool removeSuccess = (removeResult == TELEPHONY_SUCCESS);
    return std::make_pair(removeSuccess, removeResult);
}

bool InitLoop(napi_env env, uv_loop_s **loop)
{
    uint32_t napiVersion = -1;
    napi_get_version(env, &napiVersion);
#if NAPI_VERSION >= 2
    napi_status status = napi_get_uv_event_loop(env, loop);
    if (status != napi_ok) {
        TELEPHONY_LOGE("napi_get_uv_event_loop napi_status = %{public}d", status);
        return false;
    }
#endif // NAPI_VERSION >= 2
    return *loop != nullptr;
}

void InitContext(EventListener *eventListener, std::list<EventListener>::iterator listenerIterator)
{
    eventListener->env = listenerIterator->env;
    eventListener->eventType = listenerIterator->eventType;
    eventListener->slotId = listenerIterator->slotId;
    eventListener->callbackRef = listenerIterator->callbackRef;
    eventListener->index = listenerIterator->index;
}

template<typename T, typename T1, TelephonyUpdateEventType eventType>
void EventListenerHandler::HandleCallbackInfoUpdate(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }

    std::unique_ptr<T1> info = event->GetUniqueObject<T1>();
    if (info == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    for (auto itor = listenerList_.begin(); itor != listenerList_.end(); ++itor) {
        if (itor->eventType == eventType) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(itor->env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            auto context = std::make_unique<T>().release();
            if (context == nullptr) {
                TELEPHONY_LOGE("make context failed");
                break;
            }
            InitContext(context, itor);
            *context = *info;
            auto work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = (void *)context;
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
    NapiUtil::SetPropertyInt32(callStateInfo->env, callbackValue, "state", wrappedCallState);
    std::string number = NapiUtil::ToUtf8(callStateInfo->phoneNumber);
    NapiUtil::SetPropertyStringUtf8(callStateInfo->env, callbackValue, "number", number);
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
    napi_create_array(infoListUpdateInfo->env, &callbackValue);
    int listSize = static_cast<int>(infoListUpdateInfo->signalInfoList.size());
    for (int i = 0; i < listSize; ++i) {
        sptr<SignalInformation> infoItem = infoListUpdateInfo->signalInfoList[i];
        napi_value info = nullptr;
        napi_create_object(infoListUpdateInfo->env, &info);
        NapiUtil::SetPropertyInt32(
            infoListUpdateInfo->env, info, "signalType", WrapNetworkType(infoItem->GetNetworkType()));
        NapiUtil::SetPropertyInt32(infoListUpdateInfo->env, info, "signalLevel", infoItem->GetSignalLevel());
        napi_set_element(infoListUpdateInfo->env, callbackValue, i, info);
    }
    NapiReturnToJS(infoListUpdateInfo->env, infoListUpdateInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkNetworkStateUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<NetworkStateContext> networkStateUpdateInfo(static_cast<NetworkStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    napi_create_object(networkStateUpdateInfo->env, &callbackValue);
    std::string longOperatorName = networkStateUpdateInfo->networkState->GetLongOperatorName();
    std::string shortOperatorName = networkStateUpdateInfo->networkState->GetShortOperatorName();
    std::string plmnNumeric = networkStateUpdateInfo->networkState->GetPlmnNumeric();
    bool isRoaming = networkStateUpdateInfo->networkState->IsRoaming();
    int32_t regStatus = static_cast<int32_t>(networkStateUpdateInfo->networkState->GetRegStatus());
    bool isEmergency = networkStateUpdateInfo->networkState->IsEmergency();
    NapiUtil::SetPropertyStringUtf8(
        networkStateUpdateInfo->env, callbackValue, "longOperatorName", longOperatorName);
    NapiUtil::SetPropertyStringUtf8(
        networkStateUpdateInfo->env, callbackValue, "shortOperatorName", shortOperatorName);
    NapiUtil::SetPropertyStringUtf8(networkStateUpdateInfo->env, callbackValue, "plmnNumeric", plmnNumeric);
    NapiUtil::SetPropertyBoolean(networkStateUpdateInfo->env, callbackValue, "isRoaming", isRoaming);
    NapiUtil::SetPropertyInt32(networkStateUpdateInfo->env, callbackValue, "regStatus", WrapRegState(regStatus));
    NapiUtil::SetPropertyBoolean(networkStateUpdateInfo->env, callbackValue, "isEmergency", isEmergency);
    NapiReturnToJS(networkStateUpdateInfo->env, networkStateUpdateInfo->callbackRef, callbackValue);
}

void EventListenerHandler::WorkSimStateUpdated(uv_work_t *work, int status)
{
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    std::unique_ptr<SimStateContext> simStateUpdateInfo(static_cast<SimStateContext *>(work->data));
    napi_value callbackValue = nullptr;
    int32_t simState = static_cast<int32_t>(simStateUpdateInfo->simState);
    int32_t lockReason = static_cast<int32_t>(simStateUpdateInfo->reason);
    napi_create_object(simStateUpdateInfo->env, &callbackValue);
    NapiUtil::SetPropertyInt32(simStateUpdateInfo->env, callbackValue, "state", simState);
    NapiUtil::SetPropertyInt32(simStateUpdateInfo->env, callbackValue, "reason", lockReason);
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
} // namespace Telephony
} // namespace OHOS