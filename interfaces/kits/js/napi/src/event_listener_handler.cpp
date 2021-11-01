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

#include "event_listener_handler.h"

#include "inner_event.h"
#include "singleton.h"
#include "ability.h"
#include "napi_state_registry.h"
#include "napi_sim_type.h"
#include "napi_util.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "telephony_errors.h"
#include "event_listener_manager.h"
#include "napi_telephony_observer.h"
#include "telephony_state_manager.h"
#include "telephony_napi_common_error.h"
#include "telephony_callback_event_id.h"
#include "update_infos.h"
#include "update_contexts.h"

namespace OHOS {
namespace Telephony {
EventListenerHandler::EventListenerHandler()
{
    TELEPHONY_LOGD("EventListenerHandler::EventListenerHandler strat");
    auto eventRunner = AppExecFwk::EventRunner::Create();
    if (eventRunner == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    new (this) EventListenerHandler(eventRunner);
}

EventListenerHandler::EventListenerHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_REMOVE_ONCE)] =
        &EventListenerHandler::RemoveOnceEventListener;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE)] =
        &EventListenerHandler::HandleCallStateUpdated;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE)] =
        &EventListenerHandler::HandleSignalInfoUpdated;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE)] =
        &EventListenerHandler::HandleNetworkStateUpdated;
    funcMap_[ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE)] =
        &EventListenerHandler::HandleSimStateUpdated;
}

void EventListenerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("EventListenerHandler::ProcessEvent event is nullptr");
        return;
    }
    auto itor = funcMap_.find(event->GetInnerEventId());
    if (itor != funcMap_.end()) {
        auto handFunc = itor->second;
        if (handFunc != nullptr) {
            (this->*handFunc)(event);
        }
    }
}

std::pair<bool, int32_t> EventListenerHandler::AddEventListener(EventListener &eventListener)
{
    TELEPHONY_LOGD("EventListenerHandler::AddEventListener start");
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    eventListener.index = index_++;
    TELEPHONY_LOGD("eventListener.index=%{public}" PRId64 ", index_=%{public} " PRId64 ", isOnce=%{public}d",
        eventListener.index, index_, eventListener.isOnce);
    listenerList_.push_back(eventListener);
    bool registered = IsEventTypeRegistered(eventListener.eventType);
    if (!registered) {
        TELEPHONY_LOGD("EventListenerHandler::AddEventListener start to addObserver");
        auto telephonyObserver = std::make_unique<NapiTelephonyObserver>().release();
        if (telephonyObserver == nullptr) {
            TELEPHONY_LOGE("error by telephonyObserver nullptr");
            return std::make_pair(false, ERROR_DEFAULT);
        }
        sptr<TelephonyObserverBroker> observer(telephonyObserver);
        if (observer == nullptr) {
            TELEPHONY_LOGE("error by observer nullptr");
            return std::make_pair(false, ERROR_DEFAULT);
        }
        std::u16string packageName = GetBundleName(eventListener.env);
        int32_t addResult = TelephonyStateManager::AddStateObserver(
            observer, eventListener.slotId, ToUint32t(eventListener.eventType), packageName, false);
        bool addSuccess = (addResult == TELEPHONY_SUCCESS);
        if (addSuccess) {
            SetRegisterState(eventListener.eventType, true);
        }
        return std::make_pair(addSuccess, addResult);
    }
    return std::make_pair(true, ERROR_NONE);
}

std::pair<bool, int32_t> EventListenerHandler::RemoveEventListener(
    int32_t slotId, const TelephonyUpdateEventType eventType)
{
    TELEPHONY_LOGD("EventListenerHandler::RemoveEventListener start ");
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    TELEPHONY_LOGD("EventListenerHandler::RemoveEventListener after getlock");
    listenerList_.remove_if([eventType](EventListener listener) -> bool {
        auto shouldDelete = listener.eventType == eventType;
        if (shouldDelete) {
            TELEPHONY_LOGD(
                "EventListenerHandler::RemoveEventListener shouldDelete eventType = %{public}d", eventType);
            napi_delete_reference(listener.env, listener.callbackRef);
        };
        return shouldDelete;
    });
    TELEPHONY_LOGD("EventListenerHandler::RemoveEventListener after remove_iflock");
    SetRegisterState(eventType, false);
    int32_t removeResult = TelephonyStateManager::RemoveStateObserver(slotId, ToUint32t(eventType));
    bool removeSuccess = (removeResult == TELEPHONY_SUCCESS);
    return std::make_pair(removeSuccess, removeResult);
}

std::u16string EventListenerHandler::GetBundleName(napi_env env)
{
    // get global value
    napi_value global = nullptr;
    napi_get_global(env, &global);

    // get ability
    napi_value abilityObj = nullptr;
    napi_get_named_property(env, global, "ability", &abilityObj);

    // get ability pointer
    OHOS::AppExecFwk::Ability *ability = nullptr;
    napi_get_value_external(env, abilityObj, (void **)&ability);

    // get bundle path
    std::string bundleName = ability->GetBundleName();
    TELEPHONY_LOGD("getBundleName = %{public}s", bundleName.c_str());
    return NapiUtil::ToUtf16(bundleName);
}

void EventListenerHandler::SetRegisterState(const TelephonyUpdateEventType eventType, bool regist)
{
    switch (eventType) {
        case TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE: {
            registedCallStateEvent_ = regist;
            break;
        }
        case TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE: {
            registedSignalInfoEvent_ = regist;
            break;
        }
        case TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE: {
            registedNetworkStateEvent_ = regist;
            break;
        }
        case TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE: {
            registedSimStateEvent_ = regist;
            break;
        }
        default: {
            auto eventTypeValue = static_cast<uint32_t>(eventType);
            TELEPHONY_LOGE("EventListenerHandler::AddEventType mismatch eventType = %{public}u", eventTypeValue);
            break;
        }
    }
}

bool EventListenerHandler::IsEventTypeRegistered(const TelephonyUpdateEventType eventType)
{
    uint32_t eventTypeValue = static_cast<uint32_t>(eventType);
    TELEPHONY_LOGD("EventListenerHandler::IsEventTypeRegistered goal type = %{public}u", eventTypeValue);
    switch (eventType) {
        case TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE:
            return registedNetworkStateEvent_;
        case TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE:
            return registedCallStateEvent_;
        case TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE:
            return registedSignalInfoEvent_;
        case TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE:
            return registedSimStateEvent_;
        default:
            TELEPHONY_LOGE(
                "EventListenerHandler::IsEventTypeRegistered mismatch eventType = %{public}u", eventTypeValue);
            return false;
    }
}

void EventListenerHandler::RemoveOnceEventListener(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGE("EventListenerHandler::RemoveOnceEventListener start");
    if (event == nullptr) {
        TELEPHONY_LOGE("EventListenerHandler::ProcessEvent event is nullptr");
        return;
    }
    int64_t eventIndex = event->GetParam();
    TELEPHONY_LOGE("EventListenerHandler::RemoveOnceEventListener eventIndex = %{public}" PRId64 "", eventIndex);
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    TELEPHONY_LOGE("EventListenerHandler::RemoveOnceEventListener before remove_if");
    listenerList_.remove_if([eventIndex](EventListener listener) -> bool {
        bool matched = listener.index == eventIndex;
        if (matched) {
            napi_delete_reference(listener.env, listener.callbackRef);
            TELEPHONY_LOGD("EventListenerHandler::RemoveOnceEventListener the removed index = %{public}" PRId64 "",
                eventIndex);
        }
        return matched;
    });
    TELEPHONY_LOGE("EventListenerHandler::RemoveOnceEventListener end");
}

bool InitLoop(napi_env env, uv_loop_s **loop)
{
    uint32_t napiVersion = -1;
    napi_get_version(env, &napiVersion);
    TELEPHONY_LOGD("InitLoop napiVersion = %{public}u", napiVersion);
#if NAPI_VERSION >= 2
    napi_status status = napi_get_uv_event_loop(env, loop);
    TELEPHONY_LOGD("napi_get_uv_event_loop napi_status = %{public}d", status);
#endif // NAPI_VERSION >= 2
    return *loop != nullptr;
}

void InitContext(EventListener *eventListener, std::list<EventListener>::iterator listenerIterator)
{
    eventListener->env = listenerIterator->env;
    eventListener->eventType = listenerIterator->eventType;
    eventListener->slotId = listenerIterator->slotId;
    eventListener->isOnce = listenerIterator->isOnce;
    eventListener->callbackRef = listenerIterator->callbackRef;
    eventListener->index = listenerIterator->index;
}

void EventListenerHandler::HandleCallStateUpdated(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<CallStateUpdateInfo> callStateUpdateInfo = event->GetUniqueObject<CallStateUpdateInfo>();
    if (callStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    for (std::list<EventListener>::iterator listenerIterator = listenerList_.begin();
         listenerIterator != listenerList_.end(); ++listenerIterator) {
        if (listenerIterator->eventType == TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(listenerIterator->env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            auto context = std::make_unique<CallStateContext>().release();
            if (context == nullptr) {
                TELEPHONY_LOGE("make context failed");
                break;
            }
            InitContext(context, listenerIterator);
            context->callState = callStateUpdateInfo->callState;
            context->phoneNumber = callStateUpdateInfo->phoneNumber;
            auto work = std::make_unique<uv_work_t>().release();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = (void *)context;
            TELEPHONY_LOGD("HandleCallStateUpdated before uv_queue_work");
            uv_queue_work(loop, work, WorkNothing, WorkCallStateUpdated);
            TELEPHONY_LOGD("uv_queue_work end");
        }
    }
}

int32_t WrapRadioTech(RadioTech radioTechType)
{
    switch (radioTechType) {
        case RADIO_TECHNOLOGY_GSM: {
            return RADIO_TECHNOLOGY_GSM;
        }
        case RADIO_TECHNOLOGY_LTE: {
            return RADIO_TECHNOLOGY_LTE;
        }
        default: {
            return RADIO_TECHNOLOGY_UNKNOWN;
        }
    }
}

void EventListenerHandler::HandleSignalInfoUpdated(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<SignalUpdateInfo> infoListUpdateInfo = event->GetUniqueObject<SignalUpdateInfo>();
    if (infoListUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    for (std::list<EventListener>::iterator listenerIterator = listenerList_.begin();
         listenerIterator != listenerList_.end(); ++listenerIterator) {
        if (listenerIterator->eventType == TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(listenerIterator->env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            auto context = new (std::nothrow) SignalListContext;
            if (context == nullptr) {
                TELEPHONY_LOGE("new context failed");
                break;
            }
            InitContext(context, listenerIterator);
            context->signalInfoList = infoListUpdateInfo->signalInfoList;
            uv_work_t *work = new (std::nothrow) uv_work_t;
            if (work == nullptr) {
                TELEPHONY_LOGE("new work failed");
                break;
            }
            work->data = (void *)(context);
            TELEPHONY_LOGD("HandleSignalInfoUpdated before uv_queue_work");
            uv_queue_work(loop, work, WorkNothing, WorkSignalUpdated);
            TELEPHONY_LOGD("HandleSignalInfoUpdated end");
        }
    }
}

void EventListenerHandler::HandleNetworkStateUpdated(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<NetworkStateUpdateInfo> networkStateUpdateInfo =
        event->GetUniqueObject<NetworkStateUpdateInfo>();
    if (networkStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    for (std::list<EventListener>::iterator listenerIterator = listenerList_.begin();
         listenerIterator != listenerList_.end(); ++listenerIterator) {
        if (listenerIterator->eventType == TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(listenerIterator->env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            auto context = std::make_unique<NetworkStateContext>();
            if (context == nullptr) {
                TELEPHONY_LOGE("make context failed");
                break;
            }
            InitContext(context.get(), listenerIterator);
            context->networkState = networkStateUpdateInfo->networkState;
            auto work = std::make_unique<uv_work_t>();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = static_cast<void *>(context.release());
            uv_queue_work(loop, work.release(), WorkNothing, WorkNetworkStateUpdated);
        }
    }
}

void EventListenerHandler::HandleSimStateUpdated(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("event nullptr");
        return;
    }
    std::unique_ptr<SimStateUpdateInfo> simStateUpdateInfo = event->GetUniqueObject<SimStateUpdateInfo>();
    if (simStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("update info nullptr");
        return;
    }
    std::lock_guard<std::mutex> lockGuard(operatorMutex_);
    for (auto listenerIterator = listenerList_.begin(); listenerIterator != listenerList_.end();
         listenerIterator++) {
        if (listenerIterator->eventType == TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE) {
            uv_loop_s *loop = nullptr;
            if (!InitLoop(listenerIterator->env, &loop)) {
                TELEPHONY_LOGE("loop is null");
                break;
            }
            auto context = std::make_unique<SimStateContext>().release();
            if (context == nullptr) {
                TELEPHONY_LOGE("make context failed");
                break;
            }
            InitContext(context, listenerIterator);
            context->simState = simStateUpdateInfo->state;
            context->reason = simStateUpdateInfo->reason;
            auto work = std::make_unique<uv_work_t>();
            if (work == nullptr) {
                TELEPHONY_LOGE("make work failed");
                break;
            }
            work->data = static_cast<void *>(context);
            uv_queue_work(loop, work.release(), WorkNothing, WorkSimStateUpdated);
        }
    }
}

void EventListenerHandler::WorkNothing(uv_work_t *work) {}

void EventListenerHandler::WorkCallStateUpdated(uv_work_t *work, int status)
{
    TELEPHONY_LOGD("onCallStateUpdated uv_work_t start");
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    auto eventListener = static_cast<CallStateContext *>(work->data);
    auto callStateInfo = std::make_unique<CallStateContext>(*eventListener);
    DelayedSingleton<FunctionCallbackHandler>::GetInstance()->SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE), callStateInfo);
}

void EventListenerHandler::WorkSignalUpdated(uv_work_t *work, int status)
{
    TELEPHONY_LOGD("onSignalInfoUpdated uv_after_work_cb start");
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    auto eventListener = static_cast<SignalListContext *>(work->data);
    auto infoList = std::make_unique<SignalListContext>(*eventListener);
    DelayedSingleton<FunctionCallbackHandler>::GetInstance()->SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE), infoList);
}

void EventListenerHandler::WorkNetworkStateUpdated(uv_work_t *work, int status)
{
    TELEPHONY_LOGD("onNetworkStateUpdated uv_work_t start");
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    auto eventListener = static_cast<NetworkStateContext *>(work->data);
    auto networkStateUpdateInfo = std::make_unique<NetworkStateContext>(*eventListener);
    DelayedSingleton<FunctionCallbackHandler>::GetInstance()->SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE), networkStateUpdateInfo);
}

void EventListenerHandler::WorkSimStateUpdated(uv_work_t *work, int status)
{
    TELEPHONY_LOGD("WorkSimStateUpdated uv_work_t start");
    if (work == nullptr) {
        TELEPHONY_LOGE("work is null");
        return;
    }
    auto eventListener = static_cast<SimStateContext *>(work->data);
    auto simStateUpdateInfo = std::make_unique<SimStateContext>(*eventListener);
    DelayedSingleton<FunctionCallbackHandler>::GetInstance()->SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE), simStateUpdateInfo);
}
} // namespace Telephony
} // namespace OHOS