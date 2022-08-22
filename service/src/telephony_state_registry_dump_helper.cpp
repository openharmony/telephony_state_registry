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

#include "telephony_state_registry_dump_helper.h"

#include "core_service_client.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {
bool TelephonyStateRegistryDumpHelper::Dump(const std::vector<std::string> &args,
    std::vector<TelephonyStateRegistryRecord> &stateRecords, std::string &result) const
{
    result.clear();
    ShowTelephonyChangeState(result);
    return ShowTelephonyStateRegistryInfo(stateRecords, result);
}

TelephonyStateRegistryDumpHelper::TelephonyStateRegistryDumpHelper() {}

bool TelephonyStateRegistryDumpHelper::WhetherHasSimCard(const int32_t slotId) const
{
    return DelayedRefSingleton<CoreServiceClient>::GetInstance().HasSimCard(slotId);
}

bool TelephonyStateRegistryDumpHelper::ShowTelephonyStateRegistryInfo(
    std::vector<TelephonyStateRegistryRecord> &stateRecords, std::string &result) const
{
    result.append("registrations: count= ").append(std::to_string(stateRecords.size())).append("\n");
    if (!stateRecords.empty()) {
        for (const auto &item : stateRecords) {
            if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE)) {
                result.append("CellularDataConnectState Register: ");
            } else if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW)) {
                result.append("CellularDataFlow Register: ");
            } else if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE)) {
                result.append("CallState Register: ");
            } else if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE)) {
                result.append("SimState Register: ");
            } else if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS)) {
                result.append("SignalInfo Register: ");
            } else if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO)) {
                result.append("CellInfo Register: ");
            } else if (item.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE)) {
                result.append("NetworkState Register: ");
            } else {
                result.append("Unknown Subscriber: ");
            }
            result.append("\n").append("    { ");
            result.append("package: ").append(item.bundleName_);
            result.append(" pid: ").append(std::to_string(item.pid_));
            result.append(" mask: ").append(std::to_string(item.mask_));
            result.append(" slotId: ").append(std::to_string(item.slotId_));
            result.append(" }");
            result.append("\n");
        }
    }
    return true;
}

void TelephonyStateRegistryDumpHelper::ShowTelephonyChangeState(std::string &result) const
{
    std::shared_ptr<TelephonyStateRegistryService> service =
        DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("Get state registry service failed");
        return;
    }
    result.append("BindStartTime = ");
    result.append(service->GetBindStartTime());
    result.append("\n");
    result.append("BindEndTime = ");
    result.append(service->GetBindEndTime());
    result.append("\n");
    result.append("BindSpendTime = ");
    result.append(service->GetBindSpendTime());
    result.append("\n");
    result.append("ServiceRunningState = ");
    result.append(std::to_string(service->GetServiceRunningState()));
    result.append("\n");
    for (int32_t i = 0; i < SIM_SLOT_COUNT; i++) {
        if (WhetherHasSimCard(i)) {
            result.append("SlotId = ");
            result.append(std::to_string(i));
            result.append("\n");
            result.append("SimState = ");
            result.append(std::to_string(service->GetSimState(i)));
            result.append("\n");
            result.append("CardType = ");
            result.append(std::to_string(service->GetCardType(i)));
            result.append("\n");
            result.append("LockReason = ");
            result.append(std::to_string(service->GetLockReason(i)));
            result.append("\n");
            result.append("CallState = ");
            result.append(std::to_string(service->GetCallState(i)));
            result.append("\n");
            result.append("CellularDataConnectionState = ");
            result.append(std::to_string(service->GetCellularDataConnectionState(i)));
            result.append("\n");
            result.append("CellularDataFlow = ");
            result.append(std::to_string(service->GetCellularDataFlow(i)));
            result.append("\n");
            result.append("CellularDataConnectionNetworkType = ");
            result.append(std::to_string(service->GetCellularDataConnectionNetworkType(i)));
            result.append("\n");
        }
    }
}
} // namespace Telephony
} // namespace OHOS
