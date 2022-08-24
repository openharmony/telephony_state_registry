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

#ifndef TELEPHONY_STATE_REGISTRY_DUMP_HELPER_H
#define TELEPHONY_STATE_REGISTRY_DUMP_HELPER_H

#include <vector>
#include <string>

#include "telephony_state_registry_service.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
enum class DataConnectionStatus : int32_t {
    DATA_STATE_DISCONNECTED = 11,
    DATA_STATE_CONNECTING = 12,
    DATA_STATE_CONNECTED = 13,
    DATA_STATE_SUSPENDED = 14
};

enum class TelCallState {
    CALL_STATUS_ACTIVE = 0,
    CALL_STATUS_HOLDING,
    CALL_STATUS_DIALING,
    CALL_STATUS_ALERTING,
    CALL_STATUS_INCOMING,
    CALL_STATUS_WAITING,
    CALL_STATUS_DISCONNECTED,
    CALL_STATUS_DISCONNECTING,
    CALL_STATUS_IDLE,
};

enum class CellDataFlowType : int32_t {
    DATA_FLOW_TYPE_NONE = 0,
    DATA_FLOW_TYPE_DOWN = 1,
    DATA_FLOW_TYPE_UP = 2,
    DATA_FLOW_TYPE_UP_DOWN = 3,
    DATA_FLOW_TYPE_DORMANT = 4
};

class TelephonyStateRegistryDumpHelper {
public:
    explicit TelephonyStateRegistryDumpHelper();
    ~TelephonyStateRegistryDumpHelper() = default;
    bool Dump(const std::vector<std::string> &args,
        std::vector<TelephonyStateRegistryRecord> &stateRecords, std::string &result) const;

private:
    bool ShowTelephonyStateRegistryInfo(
        std::vector<TelephonyStateRegistryRecord> &stateRecords, std::string &result) const;
    void ShowTelephonyChangeState(std::string &result) const;
    bool WhetherHasSimCard(const int32_t slotId) const;
    std::string SimStateTransition(int32_t state) const;
    std::string CallStateTransition(int32_t state) const;
    std::string CardTypeTransition(int32_t type) const;
    std::string DataConnectionStateTransition(int32_t state) const;
    std::string CellularDataFlowTransition(int32_t flowData) const;
    std::string NetworkTypeTransition(int32_t type) const;
    std::string LockReasonTransition(int32_t reason) const;
};
} // namespace Telephony
} // namespace OHOS

#endif // TELEPHONY_STATE_REGISTRY_DUMP_HELPER_H
