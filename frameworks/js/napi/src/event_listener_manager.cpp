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
std::optional<int32_t> EventListenerManager::RegisterEventListener(EventListener &eventListener)
{
    return DelayedSingleton<EventListenerHandler>::GetInstance()->RegisterEventListener(eventListener);
}

std::optional<int32_t> EventListenerManager::UnregisterEventListener(
    int32_t slotId, const TelephonyUpdateEventType eventType)
{
    return DelayedSingleton<EventListenerHandler>::GetInstance()->UnregisterEventListener(slotId, eventType);
}

void EventListenerManager::RemoveListener(TelephonyUpdateEventType eventType)
{
    DelayedSingleton<EventListenerHandler>::GetInstance()->RemoveListener(eventType);
}
} // namespace Telephony
} // namespace OHOS