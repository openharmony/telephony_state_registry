/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OBSERVER_EVENT_HANDLER_H
#define OBSERVER_EVENT_HANDLER_H

#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <uv.h>

#include "event_handler.h"
#include "network_state.h"
#include "refbase.h"
#include "signal_information.h"
#include "singleton.h"
#include "telephony_observer_utils.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t EVENT_LISTENER_DIFF = -1;
constexpr int32_t EVENT_LISTENER_SAME = 0;
constexpr int32_t EVENT_LISTENER_SLOTID_AND_EVENTTYPE_SAME = 1;

class ObserverEventHandler : public AppExecFwk::EventHandler {
    DECLARE_DELAYED_SINGLETON(ObserverEventHandler)
public:
    ObserverEventHandler(const ObserverEventHandler &) = delete;
    ObserverEventHandler &operator=(const ObserverEventHandler &) = delete;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    int32_t RegisterEventListener(EventListener &eventListener);
    int32_t UnregisterEventListener(const TelephonyUpdateEventType eventType, int64_t funcId);

private:
    static std::mutex operatorMutex_;
    std::list<EventListener> listenerList_;

    bool CheckEventTypeExist(int32_t slotId, TelephonyUpdateEventType eventType);
    void SetEventListenerDeleting(std::shared_ptr<bool> isDeleting);
    void RemoveEventListenerRegister(const TelephonyUpdateEventType eventType, int64_t funcId,
        std::set<int32_t> &soltIdSet);
    void CheckRemoveStateObserver(TelephonyUpdateEventType eventType, int32_t slotId, int32_t &result);
    int32_t CheckEventListenerRegister(EventListener &eventListener);

    static void WorkUpdated(const EventListener &listener, uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkCallStateUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkSignalUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkNetworkStateUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkSimStateUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkCellInfomationUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkCellularDataConnectStateUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkCellularDataFlowUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkCfuIndicatorUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkVoiceMailMsgIndicatorUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    static void WorkIccAccountUpdated(const EventListener &listener,
        uv_work_t *work, std::unique_lock<std::mutex> &lock);
    
    template<typename D, TelephonyUpdateEventType eventType>
    void HandleCallbackInfoUpdate(const AppExecFwk::InnerEvent::Pointer &event);
    template<TelephonyUpdateEventType eventType>
    void HandleCallbackVoidUpdate(const AppExecFwk::InnerEvent::Pointer &event);
};

}
}

#endif