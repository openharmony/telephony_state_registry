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

#include <mutex>
#include <list>
#include <memory>
#include <string>
#include <map>
#include <uv.h>

#include "napi_util.h"
#include "telephony_update_event_type.h"
#include "napi/native_api.h"
#include "event_handler.h"
#include "event_runner.h"
#include "function_callback_handler.h"
#include "event_listener.h"
#include "refbase.h"
#include "signal_information.h"
#include "network_state.h"

namespace OHOS {
namespace Telephony {
class EventListenerHandler : public AppExecFwk::EventHandler {
public:
    EventListenerHandler();
    ~EventListenerHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    std::pair<bool, int32_t> AddEventListener(EventListener &eventListener);
    std::pair<bool, int32_t> RemoveEventListener(int32_t slotId, const TelephonyUpdateEventType eventType);

private:
    using HandleFuncType = void (EventListenerHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, HandleFuncType> funcMap_;
    int64_t index_ = 0;
    bool registedCallStateEvent_ = false;
    bool registedSignalInfoEvent_ = false;
    bool registedNetworkStateEvent_ = false;
    bool registedSimStateEvent_ = false;
    std::mutex operatorMutex_;
    std::list<EventListener> listenerList_;

private:
    explicit EventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    void RemoveOnceEventListener(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleCallStateUpdated(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSignalInfoUpdated(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleNetworkStateUpdated(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSimStateUpdated(const AppExecFwk::InnerEvent::Pointer &event);
    void SetRegisterState(const TelephonyUpdateEventType eventType, bool regist);
    bool IsEventTypeRegistered(const TelephonyUpdateEventType eventType);
    std::u16string GetBundleName(napi_env env);
    static void WorkNothing(uv_work_t *work);
    static void WorkCallStateUpdated(uv_work_t *work, int status);
    static void WorkSignalUpdated(uv_work_t *work, int status);
    static void WorkNetworkStateUpdated(uv_work_t *work, int status);
    static void WorkSimStateUpdated(uv_work_t *work, int status);
};
} // namespace Telephony
} // namespace OHOS
#endif // EVENT_LISTENER_MANAGER_H