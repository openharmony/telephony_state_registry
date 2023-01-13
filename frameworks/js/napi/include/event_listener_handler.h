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
constexpr int32_t EVENT_LISTENER_DIFF = -1;
constexpr int32_t EVENT_LISTENER_SAME = 0;
constexpr int32_t EVENT_LISTENER_SLOTID_AND_EVENTTYPE_SAME = 1;

class EventListenerHandler : public AppExecFwk::EventHandler {
    DECLARE_DELAYED_SINGLETON(EventListenerHandler)
public:
    EventListenerHandler(const EventListenerHandler &) = delete;
    EventListenerHandler &operator=(const EventListenerHandler &) = delete;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    int32_t RegisterEventListener(EventListener &eventListener);
    int32_t UnregisterEventListener(
        napi_env env, TelephonyUpdateEventType eventType, napi_ref ref, std::list<EventListener> &removeListenerList);
    int32_t UnregisterEventListener(
        napi_env env, TelephonyUpdateEventType eventType, std::list<EventListener> &removeListenerList);
    void UnRegisterAllListener(napi_env env);

private:
    using HandleFuncType = void (EventListenerHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<TelephonyCallbackEventId, HandleFuncType> handleFuncMap_;
    static std::map<TelephonyUpdateEventType, void (*)(uv_work_t *work)> workFuncMap_;
    static std::mutex operatorMutex_;
    std::list<EventListener> listenerList_;

private:
    bool IsCallBackRegister(napi_env env, napi_ref ref, napi_ref registeredRef) const;
    bool CheckEventTypeExist(int32_t slotId, TelephonyUpdateEventType eventType);
    void RemoveEventListenerRegister(napi_env env, TelephonyUpdateEventType eventType, napi_ref ref,
        std::list<EventListener> &removeListenerList, std::set<int32_t> &soltIdSet);
    void RemoveEventListenerRegister(napi_env env, TelephonyUpdateEventType eventType,
        std::list<EventListener> &removeListenerList, std::set<int32_t> &soltIdSet);
    void CheckRemoveStateObserver(TelephonyUpdateEventType eventType, int32_t slotId, int32_t &result);
    int32_t CheckEventListenerRegister(EventListener &eventListener);

    static void WorkCallStateUpdated(uv_work_t *work);
    static void WorkSignalUpdated(uv_work_t *work);
    static void WorkNetworkStateUpdated(uv_work_t *work);
    static void WorkSimStateUpdated(uv_work_t *work);
    static void WorkCellInfomationUpdated(uv_work_t *work);
    static void WorkCellularDataConnectStateUpdate(uv_work_t *work);
    static void WorkCellularDataFlowUpdate(uv_work_t *work);
    static void WorkUpdated(uv_work_t *work, int status);
    static void SetEventListenerDeleting(std::shared_ptr<bool> isDeleting);

    template<typename T, typename D, TelephonyUpdateEventType eventType>
    void HandleCallbackInfoUpdate(const AppExecFwk::InnerEvent::Pointer &event);
};
} // namespace Telephony
} // namespace OHOS
#endif // EVENT_LISTENER_MANAGER_H