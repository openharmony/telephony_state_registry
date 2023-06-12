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

#ifndef TELEPHONY_UPDATE_EVENT_TYPE_H
#define TELEPHONY_UPDATE_EVENT_TYPE_H

#include "telephony_observer_broker.h"

namespace OHOS {
namespace Telephony {
enum class TelephonyUpdateEventType {
    NONE_EVENT_TYPE = 0,
    EVENT_NETWORK_STATE_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE,
    EVENT_CALL_STATE_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE,
    EVENT_CELL_INFO_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO,
    EVENT_SIGNAL_STRENGTHS_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS,
    EVENT_SIM_STATE_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE,
    EVENT_DATA_CONNECTION_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE,
    EVENT_CELLULAR_DATA_FLOW_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW,
    EVENT_CFU_INDICATOR_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR,
    EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR,
    EVENT_ICC_ACCOUNT_CHANGE = TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT,
};
} // namespace Telephony
} // namespace OHOS
#endif // EVENT_TYPE_H