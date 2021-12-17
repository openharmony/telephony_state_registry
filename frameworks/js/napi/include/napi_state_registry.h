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

#ifndef NAPI_STATE_REGISTRY_H
#define NAPI_STATE_REGISTRY_H

#include <initializer_list>
#include <string>

#include "base_context.h"
#include "telephony_update_event_type.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "core_manager.h"
#include "telephony_observer_broker.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t NONE_EVENT_TYPE = 0;
constexpr int32_t LISTEN_NET_WORK_STATE = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
constexpr int32_t LISTEN_CALL_STATE = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE;
constexpr int32_t LISTEN_CELL_INFO = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
constexpr int32_t LISTEN_SIGNAL_STRENGTHS = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS;
constexpr int32_t LISTEN_SIM_STATE = TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE;
constexpr int32_t LISTEN_DATA_CONNECTION_STATE = TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE;

const std::string NET_WORK_STATE_CHANGE = "networkStateChange";
const std::string CALL_STATE_CHANGE = "callStateChange";
const std::string CELL_INFO_CHANGE = "cellInfoChange";
const std::string SIGNAL_STRENGTHS_CHANGE = "signalInfoChange";
const std::string SIM_STATE_CHANGE = "simStateChange";
const std::string DATA_CONNECTION_STATE = "dataConnectionStateChange";

constexpr int GSM = 1;
constexpr int CDMA = 2;
constexpr int LTE = 3;
constexpr int TDSCDMA = 4;

enum class CallState : int32_t {
    /**
     * Indicates an invalid state, which is used when the call state fails to be
     * obtained.
     */
    CALL_STATE_UNKNOWN = -1,

    /**
     * Indicates that there is no ongoing call.
     */
    CALL_STATE_IDLE = 0,

    /**
     * Indicates that an incoming call is ringing or waiting.
     */
    CALL_STATE_RINGING = 1,

    /**
     * Indicates that a least one call is in the dialing, active, or hold state,
     * and there is no new incoming call ringing or waiting.
     */
    CALL_STATE_OFFHOOK = 2
};

struct ObserverContext : BaseContext {
    int32_t slotId = CoreManager::DEFAULT_SLOT_ID;
    TelephonyUpdateEventType eventType = TelephonyUpdateEventType::NONE_EVENT_TYPE;
    int32_t errorCode = 0;
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_STATE_REGISTRY_H