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
#include "napi_telephony_observer.h"
#include "napi_util.h"
#include "telephony_errors.h"
#include "telephony_state_manager.h"

namespace OHOS {
namespace Telephony {
namespace {
constexpr int32_t ARRAY_SIZE = 32;
const std::map<std::string, TelephonyUpdateEventType> eventMap {
    {NET_WORK_STATE_CHANGE, TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE},
    {CALL_STATE_CHANGE, TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE},
    {SIGNAL_STRENGTHS_CHANGE, TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE},
    {SIM_STATE_CHANGE, TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE},
    {CELL_INFO_CHANGE, TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE},
};

TelephonyUpdateEventType GetEventType(const std::string &event)
{
    auto serched = eventMap.find(event);
    return (serched != eventMap.end() ? serched->second : TelephonyUpdateEventType::NONE_EVENT_TYPE);
}
} // namespace

static void NativeOn(napi_env env, void *data)
{
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    EventListener listener {
        std::move(env),
        asyncContext->eventType,
        asyncContext->slotId,
        std::move(asyncContext->callbackRef),
        0,
    };
    std::pair<bool, int32_t> resultPair = EventListenerManager::AddEventListener(listener);
    asyncContext->resolved = resultPair.first;
    asyncContext->errorCode = resultPair.second;
}

static void OnCallback(napi_env env, napi_status status, void *data)
{
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
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
    napi_value parameters[] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &parameterCount, parameters, nullptr, nullptr);

    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    std::array<char, ARRAY_SIZE> eventType {};
    napi_value object = NapiUtil::CreateUndefined(env);
    vecNapiType typeVector {napi_string, napi_function};
    bool matchResult = false;
    if (parameterCount == std::size(parameters)) {
        auto itor = typeVector.begin();
        typeVector.insert(++itor, napi_object);
        auto paraTuple = std::make_tuple(std::data(eventType), &object, &asyncContext->callbackRef);
        matchResult = MatchParameters(env, parameters, parameterCount, paraTuple, typeVector);
        if (matchResult) {
            napi_value slotId = NapiUtil::GetNamedProperty(env, object, "slotId");
            if (slotId) {
                NapiValueToCppValue(env, slotId, napi_number, &asyncContext->slotId);
            }
        }
    } else {
        auto paraTuple = std::make_tuple(std::data(eventType), &asyncContext->callbackRef);
        matchResult = MatchParameters(env, parameters, parameterCount, paraTuple, typeVector);
    }

    if (!matchResult) {
        napi_throw_error(env, nullptr, "type of input parameters error!");
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
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    std::pair<bool, int32_t> resultPair =
        EventListenerManager::RemoveEventListener(asyncContext->slotId, asyncContext->eventType);
    asyncContext->resolved = resultPair.first;
    asyncContext->errorCode = resultPair.second;
}

static void OffCallback(napi_env env, napi_status status, void *data)
{
    ObserverContext *asyncContext = static_cast<ObserverContext *>(data);
    if (!asyncContext->resolved) {
        TELEPHONY_LOGE("OffCallback error by remove observer failed");
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
    asyncContext = nullptr;
}

static napi_value Off(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 2;
    napi_value parameters[] = {nullptr, nullptr};
    napi_get_cb_info(env, info, &parameterCount, parameters, nullptr, nullptr);

    vecNapiType typeVector {napi_string, napi_function};
    std::array<char, ARRAY_SIZE> eventType {};
    std::unique_ptr<ObserverContext> asyncContext = std::make_unique<ObserverContext>();
    auto paraTuple = std::make_tuple(std::data(eventType), &asyncContext->callbackRef);
    if (parameterCount < std::size(parameters)) {
        typeVector.pop_back();
    }
    if (!MatchParameters(env, parameters, parameterCount, paraTuple, typeVector)) {
        napi_throw_error(env, nullptr, "type of input parameters error!");
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
    .nm_priv = ((void *)0),
    .reserved = {(void *)0},
};

extern "C" __attribute__((constructor)) void RegisterTelephonyObserverModule(void)
{
    napi_module_register(&_stateRegistryModule);
}
} // namespace Telephony
} // namespace OHOS
