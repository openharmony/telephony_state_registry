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

#ifndef TELEPHONY_STATE_MANAGER_H
#define TELEPHONY_STATE_MANAGER_H

#include <stdint.h>

namespace OHOS {
template<typename T>
class sptr;
namespace Telephony {
/**
 * @brief Indicates the detailed state of call.
 */
enum class CallStatus {
    /**
     * Indicates the call is unknown.
     */
    CALL_STATUS_UNKNOWN = -1,
    /**
     * Indicates the call is active.
     */
    CALL_STATUS_ACTIVE = 0,
    /**
     * Indicates the call is holding.
     */
    CALL_STATUS_HOLDING,
    /**
     * Indicates the call is dialing.
     */
    CALL_STATUS_DIALING,
    /**
     * Indicates the call is alerting.
     */
    CALL_STATUS_ALERTING,
    /**
     * Indicates the call is incoming.
     */
    CALL_STATUS_INCOMING,
    /**
     * Indicates the call is waiting.
     */
    CALL_STATUS_WAITING,
    /**
     * Indicates the call is disconnected.
     */
    CALL_STATUS_DISCONNECTED,
    /**
     * Indicates the call is disconnecting.
     */
    CALL_STATUS_DISCONNECTING,
    /**
     * Indicates the call is idle.
     */
    CALL_STATUS_IDLE,
    /**
     * Indicates the call is answered.
     */
    CALL_STATUS_ANSWERED,
};

class TelephonyObserverBroker;
class TelephonyStateManager {
public:
    static int32_t AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver,
        int32_t slotId, uint32_t mask, bool notifyNow);
    static int32_t RemoveStateObserver(int32_t slotId, uint32_t mask);
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_STATE_MANAGER_H