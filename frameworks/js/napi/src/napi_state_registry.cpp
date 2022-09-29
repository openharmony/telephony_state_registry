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
#include "telephony_errors.h"
#include "telephony_log_wrapper.h"
#include "telephony_state_manager.h"

namespace OHOS {
namespace Telephony {
namespace {
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

static void NativeOn(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("NativeOn data is nullptr");
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    TELEPHONY_LOGI("NativeOn eventType = %{public}d", asyncContext->eventType);
    EventListener listener {
        env,
        asyncContext->eventType,
        asyncContext->slotId,
        asyncContext->callbackRef,
    };
    std::optional<int32_t> result = EventListenerManager::RegisterEventListener(listener);
    asyncContext->resolved = !result.has_value();
    asyncContext->errorCode = result.value_or(ERROR_NONE);
}

static void OnCallback(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("OnCallback data is nullptr");
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OnCallback error by add observer failed");
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

static napi_value On(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &parameterCount, parameters, nullptr, nullptr);

    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    if (asyncContext == nullptr) {
        napi_throw_error(env, nullptr, "ObserverContext is nullptr!");
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
        const std::string errMsg = "type of input parameters error : " + std::to_string(errCode.value());
        napi_throw_error(env, nullptr, errMsg.c_str());
        return nullptr;
    }

    asyncContext->eventType = GetEventType(eventType.data());
    if (asyncContext->eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        return NapiUtil::HandleAsyncWork(env, asyncContext.release(), "On", NativeOn, OnCallback);
    } else {
        napi_throw_error(env, "1", "first parameter \"type\" mismatch with the observer.d.ts");
    }
    return NapiUtil::CreateUndefined(env);
}

static void NativeOff(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("NativeOff data is nullptr");
        return;
    }
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    std::optional<int32_t> result =
        EventListenerManager::UnregisterEventListener(asyncContext->slotId, asyncContext->eventType);
    asyncContext->resolved = !result.has_value();
    asyncContext->errorCode = result.value_or(ERROR_NONE);
    EventListenerManager::RemoveListener(asyncContext->eventType, asyncContext->removeListenerList);
}

static void OffCallback(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("OffCallback data is nullptr");
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
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 2;
    napi_value parameters[] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &parameterCount, parameters, nullptr, nullptr);

    std::array<char, ARRAY_SIZE> eventType {};
    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    if (asyncContext == nullptr) {
        napi_throw_error(env, nullptr, "ObserverContext is nullptr!");
        return nullptr;
    }
    auto paraTuple = std::make_tuple(std::data(eventType), &asyncContext->callbackRef);
    std::optional<NapiError> errCode = MatchParameters(env, parameters, parameterCount, paraTuple);
    if (errCode.has_value()) {
        const std::string errMsg = "type of input parameters error : " + std::to_string(errCode.value());
        napi_throw_error(env, nullptr, errMsg.c_str());
        return nullptr;
    }

    asyncContext->eventType = GetEventType(eventType.data());
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
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

extern "C" __attribute__((constructor)) void RegisterTelephonyObserverModule(void)
{
    napi_module_register(&_stateRegistryModule);
}
} // namespace Telephony
} // namespace OHOS
