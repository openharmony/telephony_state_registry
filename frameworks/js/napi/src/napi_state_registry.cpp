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

#include "napi_state_registry.h"

#include <map>
#include <utility>

#include "event_listener_manager.h"
#include "napi_parameter_util.h"
#include "napi_telephony_observer.h"
#include "napi_util.h"
#include "state_registry_errors.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"
#include "telephony_state_manager.h"

namespace OHOS {
namespace Telephony {
namespace {
constexpr const char *OBSERVER_JS_PERMISSION_ERROR_STRING =
    "Permission denied. An attempt was made to Observer "
    "On forbidden by permission : ohos.permission.GET_NETWORK_INFO or ohos.permission.LOCATION ";
constexpr int32_t ARRAY_SIZE = 64;

const std::map<std::string_view, TelephonyUpdateEventType> eventMap {
    {"networkStateChange", TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE},
    {"callStateChange", TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE},
    {"signalInfoChange", TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE},
    {"simStateChange", TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE},
    {"cellInfoChange", TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE},
    {"cellularDataConnectionStateChange", TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE},
    {"cellularDataFlowChange", TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE},
};

TelephonyUpdateEventType GetEventType(std::string_view event)
{
    auto serched = eventMap.find(event);
    return (serched != eventMap.end() ? serched->second : TelephonyUpdateEventType::NONE_EVENT_TYPE);
}
} // namespace

static inline bool IsValidSlotId(int32_t slotId)
{
    return ((slotId >= DEFAULT_SIM_SLOT_ID) && (slotId < SIM_SLOT_COUNT));
}

static void NativeOn(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("NativeOn data is nullptr");
        NapiUtil::ThrowParameterError(env);
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    if (!IsValidSlotId(asyncContext->slotId)) {
        TELEPHONY_LOGE("NativeOn slotId is invalid");
        asyncContext->errorCode = ERROR_SLOT_ID_INVALID;
        return;
    }
    TELEPHONY_LOGI("NativeOn eventType = %{public}d", asyncContext->eventType);
    std::shared_ptr<bool> isDeleting = std::make_shared<bool>(false);
    EventListener listener {
        env,
        asyncContext->eventType,
        asyncContext->slotId,
        asyncContext->callbackRef,
        isDeleting,
    };
    asyncContext->errorCode = EventListenerManager::RegisterEventListener(listener);
    if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
        asyncContext->resolved = true;
    }
}

static void OnCallback(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("OnCallback data is nullptr");
        NapiUtil::ThrowParameterError(env);
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OnCallback error by add observer failed");
        if (asyncContext->errorCode == TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED) {
            NapiUtil::ThrowError(env, JS_ERROR_TELEPHONY_PERMISSION_DENIED, OBSERVER_JS_PERMISSION_ERROR_STRING);
        } else {
            JsError error = NapiUtil::ConverErrorMessageForJs(asyncContext->errorCode);
            NapiUtil::ThrowError(env, error.errorCode, error.errorMessage);
        }
    }
    delete asyncContext;
}

static napi_value On(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[] = { nullptr, nullptr, nullptr };
    napi_get_cb_info(env, info, &parameterCount, parameters, nullptr, nullptr);

    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    if (asyncContext == nullptr) {
        TELEPHONY_LOGE("On asyncContext is nullptr.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    std::array<char, ARRAY_SIZE> eventType {};
    napi_value object = NapiUtil::CreateUndefined(env);
    std::optional<NapiError> errCode;
    if (parameterCount == std::size(parameters)) {
        auto paraTuple = std::make_tuple(std::data(eventType), &object, &asyncContext->callbackRef);
        errCode = MatchParameters(env, parameters, parameterCount, paraTuple);
        if (!errCode.has_value()) {
            napi_value slotId = NapiUtil::GetNamedProperty(env, object, "slotId");
            if (slotId) {
                NapiValueToCppValue(env, slotId, napi_number, &asyncContext->slotId);
                TELEPHONY_LOGI("state registry on slotId = %{public}d", asyncContext->slotId);
            }
        }
    } else {
        auto paraTuple = std::make_tuple(std::data(eventType), &asyncContext->callbackRef);
        errCode = MatchParameters(env, parameters, parameterCount, paraTuple);
    }

    if (errCode.has_value()) {
        TELEPHONY_LOGE("On parameter matching failed.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }

    asyncContext->eventType = GetEventType(eventType.data());
    if (asyncContext->eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        ObserverContext *observerContext = asyncContext.release();
        NativeOn(env, observerContext);
        OnCallback(env, observerContext);
    } else {
        NapiUtil::ThrowParameterError(env);
    }
    return NapiUtil::CreateUndefined(env);
}

static void NativeOff(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("NativeOff data is nullptr");
        NapiUtil::ThrowParameterError(env);
        return;
    }

    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    if (asyncContext->callbackRef == nullptr) {
        asyncContext->errorCode = EventListenerManager::UnregisterEventListener(
            env, asyncContext->eventType, asyncContext->removeListenerList);
    } else {
        asyncContext->errorCode = EventListenerManager::UnregisterEventListener(
            env, asyncContext->eventType, asyncContext->callbackRef, asyncContext->removeListenerList);
    }

    if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
        asyncContext->resolved = true;
    }
}

static void OffCallback(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("OffCallback data is nullptr");
        NapiUtil::ThrowParameterError(env);
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    for (auto listener : asyncContext->removeListenerList) {
        if (env == listener.env && listener.env != nullptr && listener.callbackRef != nullptr) {
            napi_delete_reference(listener.env, listener.callbackRef);
        }
    }
    asyncContext->removeListenerList.clear();
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OffCallback error by remove observer failed");
        if (asyncContext->errorCode == TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED) {
            NapiUtil::ThrowError(env, JS_ERROR_TELEPHONY_PERMISSION_DENIED, OBSERVER_JS_PERMISSION_ERROR_STRING);
        } else {
            JsError error = NapiUtil::ConverErrorMessageForJs(asyncContext->errorCode);
            NapiUtil::ThrowError(env, error.errorCode, error.errorMessage);
        }
    }
    if (env != nullptr && asyncContext->callbackRef != nullptr) {
        napi_delete_reference(env, asyncContext->callbackRef);
        asyncContext->callbackRef = nullptr;
    }
    delete asyncContext;
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 2;
    napi_value parameters[] = { nullptr, nullptr };
    napi_get_cb_info(env, info, &parameterCount, parameters, nullptr, nullptr);

    std::array<char, ARRAY_SIZE> eventType {};
    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    if (asyncContext == nullptr) {
        TELEPHONY_LOGE("Off asyncContext is nullptr.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }
    auto paraTuple = std::make_tuple(std::data(eventType), &asyncContext->callbackRef);
    std::optional<NapiError> errCode = MatchParameters(env, parameters, parameterCount, paraTuple);
    if (errCode.has_value()) {
        TELEPHONY_LOGE("Off parameter matching failed.");
        NapiUtil::ThrowParameterError(env);
        return nullptr;
    }

    asyncContext->eventType = GetEventType(eventType.data());
    if (asyncContext->eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        ObserverContext *observerContext = asyncContext.release();
        NativeOff(env, observerContext);
        OffCallback(env, observerContext);
    } else {
        NapiUtil::ThrowParameterError(env);
    }
    return NapiUtil::CreateUndefined(env);
}

napi_status InitEnumLockReason(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("SIM_NONE", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_NONE))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PIN", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PIN))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PUK", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PUK))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PN_PIN", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PN_PIN))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PN_PUK", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PN_PUK))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PU_PIN", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PU_PIN))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PU_PUK", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PU_PUK))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PP_PIN", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PP_PIN))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PP_PUK", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PP_PUK))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PC_PIN", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PC_PIN))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_PC_PUK", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_PC_PUK))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_SIM_PIN", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_SIM_PIN))),
        DECLARE_NAPI_STATIC_PROPERTY("SIM_SIM_PUK", GetNapiValue(env, static_cast<int32_t>(LockReason::SIM_SIM_PUK))),
    };

    constexpr size_t arrSize = sizeof(desc) / sizeof(desc[0]);
    NapiUtil::DefineEnumClassByName(env, exports, "LockReason", arrSize, desc);
    return napi_define_properties(env, exports, arrSize, desc);
}

EXTERN_C_START
napi_value InitNapiStateRegistry(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    NAPI_CALL(env, InitEnumLockReason(env, exports));
    const char *nativeStr = "InitNapiStateRegistry";
    napi_wrap(
        env, exports, static_cast<void *>(const_cast<char *>(nativeStr)),
        [](napi_env env, void *data, void *hint) { EventListenerManager::UnRegisterAllListener(env); }, nullptr,
        nullptr);
    return exports;
}
EXTERN_C_END

static napi_module _stateRegistryModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitNapiStateRegistry,
    .nm_modname = "telephony.observer",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterTelephonyObserverModule(void)
{
    napi_module_register(&_stateRegistryModule);
}
} // namespace Telephony
} // namespace OHOS
