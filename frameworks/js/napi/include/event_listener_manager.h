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

#ifndef EVENT_LISTENER_MANAGER_H
#define EVENT_LISTENER_MANAGER_H

#include <cstdint>
#include <memory>

#include "event_listener.h"
#include "event_listener_handler.h"
#include "telephony_update_event_type.h"

namespace OHOS {
namespace Telephony {
class EventListenerManager {
public:
    template<typename T, typename D>
    inline static bool SendEvent(uint32_t innerEventId, std::unique_ptr<T, D> &object, int64_t delayTime = 0)
    {
        return DelayedSingleton<EventListenerHandler>::GetInstance()->SendEvent(innerEventId, object, delayTime);
    }
    static std::optional<int32_t> RegisterEventListener(EventListener &eventListener);
    static std::optional<int32_t> UnregisterEventListener(int32_t slotId, const TelephonyUpdateEventType eventType);
    static void RemoveListener(TelephonyUpdateEventType eventType);
};
} // namespace Telephony
} // namespace OHOS
#endif // EVENT_LISTENER_MANAGER_H