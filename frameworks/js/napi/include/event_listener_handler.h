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

#ifndef EVENT_LISTENER_HANDLER_H
#define EVENT_LISTENER_HANDLER_H

#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <uv.h>

#include "event_handler.h"
#include "event_listener.h"
#include "event_runner.h"
#include "napi_util.h"
#include "network_state.h"
#include "refbase.h"
#include "signal_information.h"
#include "singleton.h"
#include "telephony_callback_event_id.h"
#include "telephony_update_event_type.h"

namespace OHOS {
namespace Telephony {
class EventListenerHandler : public AppExecFwk::EventHandler {
    DECLARE_DELAYED_SINGLETON(EventListenerHandler)
public:
    EventListenerHandler(const EventListenerHandler &) = delete;
    EventListenerHandler &operator=(const EventListenerHandler &) = delete;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    std::optional<int32_t> AddEventListener(EventListener &eventListener);
    std::optional<int32_t> RemoveEventListener(int32_t slotId, const TelephonyUpdateEventType eventType);
    void SetCallbackCompleteToListener(napi_ref ref, bool flag = true);

private:
    using HandleFuncType = void (EventListenerHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<TelephonyCallbackEventId, HandleFuncType> handleFuncMap_;
    std::map<TelephonyUpdateEventType, void (*)(uv_work_t *work, int status)> workFuncMap_;
    std::mutex operatorMutex_;
    std::list<EventListener> listenerList_;
    std::map<uint32_t, std::set<TelephonyUpdateEventType>> registerStateMap_;

private:
    void ManageRegistrants(uint32_t slotId, const TelephonyUpdateEventType eventType, bool isRegister);
    bool IsEventTypeRegistered(uint32_t slotId, const TelephonyUpdateEventType eventType) const;

    static void WorkCallStateUpdated(uv_work_t *work, int status);
    static void WorkSignalUpdated(uv_work_t *work, int status);
    static void WorkNetworkStateUpdated(uv_work_t *work, int status);
    static void WorkSimStateUpdated(uv_work_t *work, int status);
    static void WorkCellInfomationUpdated(uv_work_t *work, int status);
    static void WorkCellularDataConnectStateUpdate(uv_work_t *work, int status);
    static void WorkCellularDataFlowUpdate(uv_work_t *work, int status);

    template<typename T, typename D, TelephonyUpdateEventType eventType>
    void HandleCallbackInfoUpdate(const AppExecFwk::InnerEvent::Pointer &event);
};
} // namespace Telephony
} // namespace OHOS
#endif // EVENT_LISTENER_MANAGER_H