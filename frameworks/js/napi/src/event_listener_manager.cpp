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

#include "event_listener_manager.h"

#include "telephony_update_event_type.h"

namespace OHOS {
namespace Telephony {
int32_t EventListenerManager::RegisterEventListener(EventListener &eventListener)
{
    auto handler = DelayedSingleton<EventListenerHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return handler->RegisterEventListener(eventListener);
}

int32_t EventListenerManager::UnregisterEventListener(
    napi_env env, const TelephonyUpdateEventType eventType, napi_ref ref, std::list<EventListener> &removeListenerList)
{
    auto handler = DelayedSingleton<EventListenerHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return handler->UnregisterEventListener(env, eventType, ref, removeListenerList);
}

int32_t EventListenerManager::UnregisterEventListener(
    napi_env env, TelephonyUpdateEventType eventType, std::list<EventListener> &removeListenerList)
{
    auto handler = DelayedSingleton<EventListenerHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return handler->UnregisterEventListener(env, eventType, removeListenerList);
}

void EventListenerManager::UnRegisterAllListener(napi_env env)
{
    auto handler = DelayedSingleton<EventListenerHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return;
    }
    handler->UnRegisterAllListener(env);
}
} // namespace Telephony
} // namespace OHOS
