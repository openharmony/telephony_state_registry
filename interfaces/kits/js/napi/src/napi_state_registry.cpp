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

#include "string_ex.h"

#include "telephony_state_manager.h"
#include "event_listener.h"
#include "napi_telephony_observer.h"
#include "kit_state_registry_hilog_wrapper.h"

namespace OHOS {
namespace TelephonyNapi {
constexpr int ZERO = 0;
constexpr int ONE = 1;
constexpr int TWO = 2;
constexpr int BUF_SIZE = 31;
constexpr int EVENT_TYPE_LENGTH = 32;
static std::unique_ptr<TelephonyState::TelephonyStateManager> g_telephonyStateManager;
static std::list<EventListener> g_eventListenerList;
static uint32_t g_eventMasks = ZERO;
static bool InitTelephonyStateManager()
{
    if (g_telephonyStateManager == nullptr) {
        g_telephonyStateManager = std::make_unique<TelephonyState::TelephonyStateManager>();
    }
    return g_telephonyStateManager != nullptr;
}

static int32_t GetDefaultSlotId()
{
    return ZERO;
}

static void AddStateObserver(uint32_t eventType, int32_t slotId, const std::u16string &package)
{
    if ((eventType & g_eventMasks) == ZERO) {
        g_eventMasks += eventType;
        sptr<TelephonyState::TelephonyObserverBroker> observer =
            new NapiTelephonyObserver(eventType, g_eventListenerList);
        if ((observer != nullptr) && InitTelephonyStateManager()) {
            g_telephonyStateManager->AddStateObserver(observer, slotId, eventType, package, false);
            HILOG_DEBUG("Exec ObserverOnce AddStateObserver");
        }
    }
}

static void HasSlotIdProperty(
    bool hasSlotIdProperty, napi_env env, int32_t slotId, const std::string &slotIdStr, napi_value value)
{
    HILOG_DEBUG(
        "Exec ObserverOn argc == THREE_PARAMTER "
        "napi_has_named_property end hasSlotIdProperty = %{public}d",
        hasSlotIdProperty);
    if (hasSlotIdProperty) {
        napi_value propertyName = nullptr;
        napi_value slotIdValue = nullptr;
        char *nameChars = (char *)slotIdStr.data();
        napi_create_string_utf8(env, nameChars, std::strlen(nameChars), &propertyName);
        napi_get_property(env, value, propertyName, &slotIdValue);
        napi_get_value_int32(env, slotIdValue, &(slotId));
        HILOG_DEBUG("Exec ObserverOn argc == TelephonyNapi::THREE_PARAMTER  hasSlotIdProperty end ");
    }
}

static napi_value ObserverOn(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("Exec ObserverOn Start");
    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc == TWO_PARAMTER) {
        NAPI_ASSERT(
            env, NapiTelephonyObserver::MatchParamters(env, argv, {napi_string, napi_function}), "type mismatch");
    } else if (argc == THREE_PARAMTER) {
        NAPI_ASSERT(env,
            NapiTelephonyObserver::MatchParamters(env, argv, {napi_string, napi_object, napi_function}),
            "type mismatch");
    } else {
        NAPI_ASSERT(env, false, "type mismatch");
    }
    napi_ref callbackRef = nullptr;
    char eventTypeChars[EVENT_TYPE_LENGTH];
    size_t eventTypeCharsSize;
    int32_t slotId = GetDefaultSlotId();
    napi_get_value_string_utf8(env, argv[ZERO], eventTypeChars, BUF_SIZE, &eventTypeCharsSize);
    std::string eventTypeStr(eventTypeChars, eventTypeCharsSize);
    if (argc == TWO_PARAMTER) {
        napi_create_reference(env, argv[ONE], ONE_PARAMTER, &callbackRef);
    } else if (argc == THREE_PARAMTER) {
        bool hasSlotIdProperty = false;
        std::string slotIdStr = "slotId";
        HILOG_DEBUG("Exec ObserverOn argc == TelephonyNapi::THREE_PARAMTER napi_has_named_property begin");
        napi_has_named_property(env, argv[ONE], slotIdStr.data(), &hasSlotIdProperty);
        HasSlotIdProperty(hasSlotIdProperty, env, slotId, slotIdStr, argv[ONE]);
        napi_create_reference(env, argv[TWO], ONE_PARAMTER, &callbackRef);
        HILOG_DEBUG("Exec ObserverOn argc == THREE_PARAMTER end");
    }
    napi_value result = nullptr;
    uint32_t eventType = NapiTelephonyObserver::GetEventType(eventTypeStr);
    if (eventType != NONE_EVENT_TYPE) {
        EventListener listener = {env, eventType, false, thisVar, callbackRef, slotId};
        g_eventListenerList.push_back(listener);
        HILOG_DEBUG("Exec ObserverOn after push_back size = %{public}d", g_eventListenerList.size());
    }
    std::string package("telephony.state_registry.on");
    std::u16string packageForU16 = Str8ToStr16(package);
    AddStateObserver(eventType, slotId, packageForU16);
    HILOG_DEBUG("Exec ObserverOn End");
    return result;
}

static napi_value ObserverOnce(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("Exec ObserverOnce Start");
    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc == TWO_PARAMTER) {
        NAPI_ASSERT(
            env, NapiTelephonyObserver::MatchParamters(env, argv, {napi_string, napi_function}), "type mismatch");
    } else if (argc == THREE_PARAMTER) {
        NAPI_ASSERT(env,
            NapiTelephonyObserver::MatchParamters(env, argv, {napi_string, napi_object, napi_function}),
            "type mismatch");
    } else {
        NAPI_ASSERT(env, false, "type mismatch");
    }
    napi_ref callbackRef = nullptr;
    char eventTypeChars[EVENT_TYPE_LENGTH];
    size_t eventTypeCharsSize;
    int32_t slotId = GetDefaultSlotId();
    napi_get_value_string_utf8(env, argv[ZERO], eventTypeChars, BUF_SIZE, &eventTypeCharsSize);
    if (argc == TWO_PARAMTER) {
        napi_create_reference(env, argv[ONE], ONE_PARAMTER, &callbackRef);
    } else if (argc == THREE_PARAMTER) {
        bool hasSlotIdProperty = false;
        std::string slotIdStr = "slotId";
        napi_has_named_property(env, argv[ONE], slotIdStr.data(), &hasSlotIdProperty);
        HasSlotIdProperty(hasSlotIdProperty, env, slotId, slotIdStr, argv[ONE]);
        napi_create_reference(env, argv[TWO], ONE_PARAMTER, &callbackRef);
    }
    napi_value result = nullptr;
    uint32_t eventType = NapiTelephonyObserver::GetEventType(eventTypeChars);
    if (eventType != NONE_EVENT_TYPE) {
        EventListener listener = {env, eventType, true, thisVar, callbackRef, slotId};
        g_eventListenerList.push_back(listener);
        HILOG_DEBUG("Exec ObserverOnce after push_back size = %{public}d", g_eventListenerList.size());
    }
    std::string package("telephony.test.once");
    std::u16string packageForU16 = Str8ToStr16(package);
    AddStateObserver(eventType, slotId, packageForU16);
    HILOG_DEBUG("Exec ObserverOnce End");
    return result;
}

static napi_value ObserverOff(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("Exec ObserverOff start");
    size_t argc = 3;
    napi_value argv[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    if (argc == ONE_ARGUMENT) {
        NAPI_ASSERT(env, NapiTelephonyObserver::MatchParamters(env, argv, {napi_string}), "type mismatch");
    } else if (argc == TWO_PARAMTER) {
        NAPI_ASSERT(
            env, NapiTelephonyObserver::MatchParamters(env, argv, {napi_string, napi_function}), "type mismatch");
    } else {
        NAPI_ASSERT(env, false, "type mismatch");
    }
    napi_value result = nullptr;
    char eventTypeChars[EVENT_TYPE_LENGTH];
    size_t eventTypeCharsSize;
    napi_get_value_string_utf8(env, argv[ZERO], eventTypeChars, BUF_SIZE, &eventTypeCharsSize);
    int32_t eventType = NapiTelephonyObserver::GetEventType(eventTypeChars);
    HILOG_DEBUG("Exec ObserverOff start eventType = %{public}d", eventType);
    if (eventType != NONE_EVENT_TYPE) {
        HILOG_DEBUG("Exec ObserverOff before InitTelephonyStateManager");
        if (InitTelephonyStateManager()) {
            HILOG_DEBUG("Exec ObserverOff before Traversal size = %{public}d", g_eventListenerList.size());
            for (std::list<EventListener>::iterator it = g_eventListenerList.begin();
                 it != g_eventListenerList.end(); it++) {
                HILOG_DEBUG("Exec ObserverOff in the for Traversal");
                if (it->eventType == eventType) {
                    HILOG_DEBUG(
                        "Exec ObserverOff before RemoveStateObserver eventType = %{public}d", it->eventType);
                    g_telephonyStateManager->RemoveStateObserver(it->slotId, it->eventType);
                    HILOG_DEBUG("Exec ObserverOff after RemoveStateObserver eventType = %{public}d", it->eventType);
                }
                HILOG_DEBUG("Exec ObserverOff in the for Traversal end");
            }
        }
        HILOG_DEBUG("Exec ObserverOff before remove_if eventList");
        g_eventListenerList.remove_if(
            [eventType](EventListener listener) -> bool { return listener.eventType == eventType; });
        HILOG_DEBUG("Exec ObserverOff after remove_if size = %{public}d", g_eventListenerList.size());
    }
    HILOG_DEBUG("Exec ObserverOff end");
    return result;
}

EXTERN_C_START
napi_value InitNapiStateRegistry(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", ObserverOn),
        DECLARE_NAPI_FUNCTION("once", ObserverOnce), DECLARE_NAPI_FUNCTION("off", ObserverOff)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[ZERO]), desc));
    return exports;
}
EXTERN_C_END

static napi_module _stateRegistryModule = {
    .nm_version = ONE,
    .nm_flags = ZERO,
    .nm_filename = nullptr,
    .nm_register_func = InitNapiStateRegistry,
    .nm_modname = "libtelephony_observer.z.so",
    .nm_priv = ((void *)ZERO),
    .reserved = {(void *)ZERO}
};

extern "C" __attribute__((constructor)) void RegisterTelephonyObserverModule(void)
{
    napi_module_register(&_stateRegistryModule);
}
} // namespace TelephonyNapi
} // namespace OHOS
