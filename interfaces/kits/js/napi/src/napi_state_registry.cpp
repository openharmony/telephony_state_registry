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

#include <utility>

#include "napi_util.h"
#include "telephony_state_manager.h"
#include "event_listener.h"
#include "event_listener_manager.h"
#include "napi_telephony_observer.h"
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t DEFAULT_REF_COUNT = 1;

static bool MatchEventType(const std::string &type, const std::string &goalTypeStr)
{
    return goalTypeStr.compare(type) == 0;
}

static TelephonyUpdateEventType GetEventType(const std::string &type)
{
    if (MatchEventType(type, NET_WORK_STATE_CHANGE)) {
        return TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE;
    }
    if (MatchEventType(type, CALL_STATE_CHANGE)) {
        return TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE;
    }
    if (MatchEventType(type, SIGNAL_STRENGTHS_CHANGE)) {
        return TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE;
    }
    if (MatchEventType(type, SIM_STATE_CHANGE)) {
        return TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE;
    }
    return TelephonyUpdateEventType::NONE_EVENT_TYPE;
}

static std::pair<bool, std::string> MatchOnParameters(
    napi_env env, const napi_value parameters[], size_t parameterCount)
{
    if (parameterCount == 2) {
        return std::make_pair(NapiUtil::MatchParameters(env, parameters, {napi_string, napi_function}),
            "type mismatch: expect string, function");
    } else if (parameterCount == 3) {
        bool typeMatch = false;
        bool parameterTypeMatch =
            NapiUtil::MatchParameters(env, parameters, {napi_string, napi_object, napi_function});
        if (parameterTypeMatch) {
            typeMatch = NapiUtil::HasNamedTypeProperty(env, parameters[1], napi_number, "slotId");
        }
        return std::make_pair(typeMatch, "type mismatch: expect string, object, function");
    } else {
        return std::make_pair(false, "type mismatch:expect 2 or 3 parameters");
    }
}

static void NativeOn(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeOn start");
    auto asyncContext = static_cast<ObserverContext *>(data);
    TELEPHONY_LOGD("NativeOn eventType = %{public}d", asyncContext->eventType);
    EventListener listener {
        env, asyncContext->eventType, asyncContext->slotId, false, asyncContext->callbackRef, 0};
    std::pair<bool, int32_t> resultPair = EventListenerManager::AddEventListener(listener);
    asyncContext->resolved = resultPair.first;
    asyncContext->errorCode = resultPair.second;
}

static void OnCallback(napi_env env, napi_status status, void *data)
{
    TELEPHONY_LOGD("OnCallback start");
    auto asyncContext = static_cast<ObserverContext *>(data);
    TELEPHONY_LOGD("NativeOn end resolved = %{public}d , errorCode = %{public}d", asyncContext->resolved,
        asyncContext->errorCode);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OnCallback error by add observer failed");
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
    asyncContext = nullptr;
}

static napi_value On(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    std::pair<bool, std::string> matchResult = MatchOnParameters(env, parameters, parameterCount);
    NAPI_ASSERT(env, matchResult.first, "type mismatch");
    auto asyncContext = std::make_unique<ObserverContext>();
    std::string eventTypeStr = NapiUtil::GetStringFromValue(env, parameters[0]);
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    } else if (parameterCount == 3) {
        napi_value slotIdValue = NapiUtil::GetNamedProperty(env, parameters[1], "slotId");
        if (slotIdValue != nullptr) {
            NapiValueConverted(env, slotIdValue, &asyncContext->slotId);
        }
        napi_create_reference(env, parameters[2], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    asyncContext->eventType = GetEventType(eventTypeStr);
    if (asyncContext->eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "On", NativeOn, OnCallback);
    } else {
        napi_throw_error(env, "1", "first parameter \"type\" mismatch with the observer.d.ts");
    }
    return NapiUtil::CreateUndefined(env);
}

static void NativeOnce(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeOnce start");
    auto asyncContext = static_cast<ObserverContext *>(data);
    TELEPHONY_LOGD("NativeOnce eventType = %{public}d", asyncContext->eventType);
    EventListener listener {env, asyncContext->eventType, asyncContext->slotId, true, asyncContext->callbackRef, 0};
    std::pair<bool, int32_t> resultPair = EventListenerManager::AddEventListener(listener);
    asyncContext->resolved = resultPair.first;
    asyncContext->errorCode = resultPair.second;
}

static void OnceCallback(napi_env env, napi_status status, void *data)
{
    TELEPHONY_LOGD("OnceCallback start");
    auto asyncContext = static_cast<ObserverContext *>(data);
    TELEPHONY_LOGD("OnceCallback resolved = %{public}d , errorCode = %{public}d", asyncContext->resolved,
        asyncContext->errorCode);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OnceCallback error by add observer failed");
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
    asyncContext = nullptr;
    TELEPHONY_LOGD("OnceCallback end");
}

static napi_value Once(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    std::pair<bool, std::string> matchResult = MatchOnParameters(env, parameters, parameterCount);
    NAPI_ASSERT(env, matchResult.first, "type mismatch");
    auto asyncContext = std::make_unique<ObserverContext>();
    std::string eventTypeStr = NapiUtil::GetStringFromValue(env, parameters[0]);
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    } else if (parameterCount == 3) {
        napi_value slotIdValue = NapiUtil::GetNamedProperty(env, parameters[1], "slotId");
        if (slotIdValue != nullptr) {
            NapiValueConverted(env, slotIdValue, &asyncContext->slotId);
        }
        napi_create_reference(env, parameters[2], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    asyncContext->eventType = GetEventType(eventTypeStr);
    if (asyncContext->eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "Once", NativeOnce, OnceCallback);
    } else {
        napi_throw_error(env, "1", "first parameter \"type\" mismatch with the observer.d.ts");
    }
    return NapiUtil::CreateUndefined(env);
}

static void NativeOff(napi_env env, void *data)
{
    TELEPHONY_LOGD("NativeOff start");
    auto asyncContext = static_cast<ObserverContext *>(data);
    std::pair<bool, int32_t> resultPair =
        EventListenerManager::RemoveEventListener(asyncContext->slotId, asyncContext->eventType);
    asyncContext->resolved = resultPair.first;
    asyncContext->errorCode = resultPair.second;
}

static void OffCallback(napi_env env, napi_status status, void *data)
{
    TELEPHONY_LOGD("OffCallback start");
    auto asyncContext = static_cast<ObserverContext *>(data);
    TELEPHONY_LOGD("OffCallback resolved = %{public}d , errorCode = %{public}d", asyncContext->resolved,
        asyncContext->errorCode);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OffCallback error by remove observer failed");
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
    asyncContext = nullptr;
    TELEPHONY_LOGD("OffCallback end");
}

static std::pair<bool, std::string> MatchOffParameters(
    napi_env env, const napi_value parameters[], const size_t parameterCount)
{
    if (parameterCount == 1) {
        return std::make_pair(
            NapiUtil::MatchParameters(env, parameters, {napi_string}), "type mismatch: expect string");
    } else if (parameterCount == 2) {
        return std::make_pair(NapiUtil::MatchParameters(env, parameters, {napi_string, napi_function}),
            "type mismatch: expect string, function");
    } else {
        return std::make_pair(false, "type mismatch:expect 1 or 2 parameters");
    }
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 2;
    napi_value parameters[2] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    std::pair<bool, std::string> matchResult = MatchOffParameters(env, parameters, parameterCount);
    NAPI_ASSERT(env, matchResult.first, "type mismatch");
    auto asyncContext = std::make_unique<ObserverContext>();
    std::string eventTypeStr = NapiUtil::GetStringFromValue(env, parameters[0]);
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], DEFAULT_REF_COUNT, &asyncContext->callbackRef);
    }
    asyncContext->eventType = GetEventType(eventTypeStr);
    if (asyncContext->eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "Off", NativeOff, OffCallback);
    } else {
        napi_throw_error(env, "1", "first parameter \"type\" mismatch with the @ohos.telephony.observer.d.ts");
    return NapiUtil::CreateUndefined(env);
    }
}

EXTERN_C_START
napi_value InitNapiStateRegistry(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("once", Once),
        DECLARE_NAPI_FUNCTION("off", Off),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
EXTERN_C_END

static napi_module _stateRegistryModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitNapiStateRegistry,
    .nm_modname = "telephony.observer",
    .nm_priv = ((void *)0),
    .reserved = {(void *)0},
};

extern "C" __attribute__((constructor)) void RegisterTelephonyObserverModule(void)
{
    napi_module_register(&_stateRegistryModule);
}
} // namespace Telephony
} // namespace OHOS
