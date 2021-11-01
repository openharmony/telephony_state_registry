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

#include "function_callback_handler.h"

#include "telephony_callback_event_id.h"
#include "napi_sim_type.h"
#include "napi_radio_types.h"
#include "napi_state_registry.h"
#include "call_manager_inner_type.h"
#include "update_contexts.h"
#include "event_listener_handler.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
FunctionCallbackHandler::FunctionCallbackHandler()
{
    TELEPHONY_LOGD("FunctionCallbackHandler::FunctionCallbackHandler strat");
    auto eventRunner = AppExecFwk::EventRunner::Create();
    if (eventRunner == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    new (this) FunctionCallbackHandler(eventRunner);
}

FunctionCallbackHandler::FunctionCallbackHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE)] =
        &FunctionCallbackHandler::CallbackCallState;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE)] =
        &FunctionCallbackHandler::CallbackSignalInfo;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE)] =
        &FunctionCallbackHandler::CallbackNetworkState;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE)] =
        &FunctionCallbackHandler::CallbackSimState;
}

void FunctionCallbackHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("FunctionCallbackHandler::ProcessEvent event is nullptr");
        return;
    }
    auto itor = funcMap_.find(event->GetInnerEventId());
    if (itor != funcMap_.end()) {
        auto handFunc = itor->second;
        if (handFunc != nullptr) {
            (this->*handFunc)(event);
        }
    }
}

static int32_t WrapRegState(int32_t nativeState)
{
    switch (nativeState) {
        case REG_STATE_NO_SERVICE:
        case REG_STATE_SEARCH: {
            return REGISTRATION_STATE_NO_SERVICE;
        }
        case REG_STATE_IN_SERVICE: {
            return REGISTRATION_STATE_IN_SERVICE;
        }
        case REG_STATE_EMERGENCY_ONLY: {
            return REGISTRATION_STATE_EMERGENCY_CALL_ONLY;
        }
        case REG_STATE_UNKNOWN: {
            return REGISTRATION_STATE_POWER_OFF;
        }
        default:
            return REGISTRATION_STATE_POWER_OFF;
    }
}

static int32_t WrapCallState(int32_t callState)
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

void FunctionCallbackHandler::CallbackCallState(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<CallStateContext> callStateUpdateInfo = event->GetUniqueObject<CallStateContext>();
    if (callStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    napi_value callbackValue = nullptr;
    napi_create_object(callStateUpdateInfo->env, &callbackValue);
    int32_t wrappedCallState = WrapCallState(callStateUpdateInfo->callState);
    NapiUtil::SetPropertyInt32(callStateUpdateInfo->env, callbackValue, "state", wrappedCallState);
    std::string phoneNumberStr = NapiUtil::ToUtf8(callStateUpdateInfo->phoneNumber);
    NapiUtil::SetPropertyStringUtf8(callStateUpdateInfo->env, callbackValue, "number", phoneNumberStr);
    napi_value callbackFunc = nullptr;
    napi_get_reference_value(callStateUpdateInfo->env, callStateUpdateInfo->callbackRef, &callbackFunc);
    napi_value callbackValues[] = {callbackValue};
    napi_value recv = nullptr;
    napi_get_undefined(callStateUpdateInfo->env, &recv);
    napi_value callbackResult = nullptr;
    napi_call_function(
        callStateUpdateInfo->env, recv, callbackFunc, std::size(callbackValues), callbackValues, &callbackResult);
    if (callStateUpdateInfo->isOnce) {
        auto eventId = ToUint32t(TelephonyCallbackEventId::EVENT_REMOVE_ONCE);
        TELEPHONY_LOGD("CallbackCallState isOnce eventId = %{public}u , index = %{public}" PRId64 "", eventId,
            callStateUpdateInfo->index);
        DelayedSingleton<EventListenerHandler>::GetInstance()->SendEvent(
            AppExecFwk::InnerEvent::Get(eventId, callStateUpdateInfo->index));
    }
}

static int32_t WrapNetworkType(SignalInformation::NetworkType networkType)
{
    NetworkType wrapedNetworkType = NetworkType::NETWORK_TYPE_UNKNOWN;
    int type = static_cast<int>(networkType);
    switch (type) {
        case GSM: {
            wrapedNetworkType = NetworkType::NETWORK_TYPE_GSM;
            break;
        }
        case LTE: {
            wrapedNetworkType = NetworkType::NETWORK_TYPE_LTE;
            break;
        }
        default: {
            break;
        }
    }
    return static_cast<int32_t>(wrapedNetworkType);
}

void FunctionCallbackHandler::CallbackSignalInfo(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<SignalListContext> infoListUpdateInfo = event->GetUniqueObject<SignalListContext>();
    if (infoListUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    napi_value callbackValue = nullptr;
    napi_create_array(infoListUpdateInfo->env, &callbackValue);
    int listSize = static_cast<int>(infoListUpdateInfo->signalInfoList.size());
    TELEPHONY_LOGD("OnSignalInfoUpdated listSize = %{public}d", listSize);
    for (int i = 0; i < listSize; ++i) {
        sptr<SignalInformation> infoItem = infoListUpdateInfo->signalInfoList[i];
        napi_value info = nullptr;
        napi_create_object(infoListUpdateInfo->env, &info);
        NapiUtil::SetPropertyInt32(
            infoListUpdateInfo->env, info, "signalType", WrapNetworkType(infoItem->GetNetworkType()));
        NapiUtil::SetPropertyInt32(infoListUpdateInfo->env, info, "signalLevel", infoItem->GetSignalLevel());
        napi_set_element(infoListUpdateInfo->env, callbackValue, i, info);
    }
    napi_value callbackFunc = nullptr;
    napi_get_reference_value(infoListUpdateInfo->env, infoListUpdateInfo->callbackRef, &callbackFunc);
    napi_value callbackValues[] = {callbackValue};
    napi_value recv = nullptr;
    napi_get_undefined(infoListUpdateInfo->env, &recv);
    napi_value callbackResult = nullptr;
    napi_call_function(
        infoListUpdateInfo->env, recv, callbackFunc, std::size(callbackValues), callbackValues, &callbackResult);
    if (infoListUpdateInfo->isOnce) {
        auto eventId = ToUint32t(TelephonyCallbackEventId::EVENT_REMOVE_ONCE);
        TELEPHONY_LOGD("CallbackSignalInfo isOnce eventId = %{public}u , index = %{public}" PRId64 "", eventId,
            infoListUpdateInfo->index);
        DelayedSingleton<EventListenerHandler>::GetInstance()->SendEvent(
            AppExecFwk::InnerEvent::Get(eventId, infoListUpdateInfo->index));
        TELEPHONY_LOGD("CallbackSignalInfo end");
    }
}

void FunctionCallbackHandler::CallbackNetworkState(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<NetworkStateContext> networkStateUpdateInfo = event->GetUniqueObject<NetworkStateContext>();
    if (networkStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    napi_value callbackValue = nullptr;
    napi_create_object(networkStateUpdateInfo->env, &callbackValue);
    NapiUtil::SetPropertyStringUtf8(networkStateUpdateInfo->env, callbackValue, "longOperatorName",
        networkStateUpdateInfo->networkState->GetLongOperatorName());
    NapiUtil::SetPropertyStringUtf8(networkStateUpdateInfo->env, callbackValue, "shortOperatorName",
        networkStateUpdateInfo->networkState->GetShortOperatorName());
    NapiUtil::SetPropertyStringUtf8(networkStateUpdateInfo->env, callbackValue, "plmnNumeric",
        networkStateUpdateInfo->networkState->GetPlmnNumeric());
    NapiUtil::SetPropertyBoolean(networkStateUpdateInfo->env, callbackValue, "isRoaming",
        networkStateUpdateInfo->networkState->IsRoaming());
    NapiUtil::SetPropertyInt32(networkStateUpdateInfo->env, callbackValue, "regStatus",
        WrapRegState(networkStateUpdateInfo->networkState->GetRegStatus()));
    NapiUtil::SetPropertyBoolean(networkStateUpdateInfo->env, callbackValue, "isEmergency",
        networkStateUpdateInfo->networkState->IsEmergency());
    napi_value callbackFunc = nullptr;
    napi_get_reference_value(networkStateUpdateInfo->env, networkStateUpdateInfo->callbackRef, &callbackFunc);
    napi_value callbackValues[] = {callbackValue};
    napi_value recv = nullptr;
    napi_get_undefined(networkStateUpdateInfo->env, &recv);
    napi_value callbackResult = nullptr;
    napi_call_function(networkStateUpdateInfo->env, recv, callbackFunc, std::size(callbackValues), callbackValues,
        &callbackResult);
    if (networkStateUpdateInfo->isOnce) {
        auto eventId = ToUint32t(TelephonyCallbackEventId::EVENT_REMOVE_ONCE);
        TELEPHONY_LOGD("FunctionCallbackHandler isOnce eventId = %{public}u , index = %{public}" PRId64 "", eventId,
            networkStateUpdateInfo->index);
        DelayedSingleton<EventListenerHandler>::GetInstance()->SendEvent(
            AppExecFwk::InnerEvent::Get(eventId, networkStateUpdateInfo->index));
        TELEPHONY_LOGD("FunctionCallbackHandler end");
    }
}

static int32_t WrapSimState(int32_t simState)
{
    switch (simState) {
        case ExternalState::EX_READY:
            return static_cast<int32_t>(SimState::SIM_STATE_READY);
        case ExternalState::EX_PIN_LOCKED:
        case ExternalState::EX_PUK_LOCKED:
        case ExternalState::EX_SIMLOCK:
        case ExternalState::EX_BLOCKED_PERM:
            return static_cast<int32_t>(SimState::SIM_STATE_LOCKED);
        case ExternalState::EX_ABSENT:
            return static_cast<int32_t>(SimState::SIM_STATE_NOT_PRESENT);
        case ExternalState::EX_UNREADY:
        case ExternalState::EX_ICC_ERROR:
        case ExternalState::EX_ICC_RESTRICTED:
            return static_cast<int32_t>(SimState::SIM_STATE_NOT_READY);
        case ExternalState::EX_LOADED:
            return static_cast<int32_t>(SimState::SIM_STATE_LOADED);
        default:
            return static_cast<int32_t>(SimState::SIM_STATE_UNKNOWN);
    }
}

static int32_t WrapLockReason(int32_t simState)
{
    if (simState == ExternalState::EX_PIN_LOCKED) {
        return LockReason::SIM_PIN;
    } else if (simState == ExternalState::EX_PUK_LOCKED) {
        return LockReason::SIM_PUK;
    }
    return LockReason::SIM_NONE;
}

void FunctionCallbackHandler::CallbackSimState(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<SimStateContext> simStateUpdateInfo = event->GetUniqueObject<SimStateContext>();
    if (simStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    napi_value callbackValue = nullptr;
    int32_t wrappedSimState = WrapSimState(simStateUpdateInfo->simState);
    int32_t lockReason = WrapLockReason(simStateUpdateInfo->simState);
    TELEPHONY_LOGD("WorkSimStateUpdated simState %{public}d, LockReason %{public}d", wrappedSimState, lockReason);
    napi_create_object(simStateUpdateInfo->env, &callbackValue);
    NapiUtil::SetPropertyInt32(simStateUpdateInfo->env, callbackValue, "state", wrappedSimState);
    NapiUtil::SetPropertyInt32(simStateUpdateInfo->env, callbackValue, "reason", lockReason);
    napi_value callbackFunc = nullptr;
    napi_get_reference_value(simStateUpdateInfo->env, simStateUpdateInfo->callbackRef, &callbackFunc);
    napi_value callbackValues[] = {callbackValue};
    napi_value recv = nullptr;
    napi_get_undefined(simStateUpdateInfo->env, &recv);
    napi_value callbackResult = nullptr;
    napi_call_function(
        simStateUpdateInfo->env, recv, callbackFunc, std::size(callbackValues), callbackValues, &callbackResult);
    if (simStateUpdateInfo->isOnce) {
        auto eventId = ToUint32t(TelephonyCallbackEventId::EVENT_REMOVE_ONCE);
        TELEPHONY_LOGD("FunctionCallbackHandler isOnce eventId = %{public}d , index = %{public}" PRId64 "", eventId,
            simStateUpdateInfo->index);
        DelayedSingleton<EventListenerHandler>::GetInstance()->SendEvent(
            AppExecFwk::InnerEvent::Get(eventId, simStateUpdateInfo->index));
        TELEPHONY_LOGD("FunctionCallbackHandler end");
    }
}
} // namespace Telephony
} // namespace OHOS