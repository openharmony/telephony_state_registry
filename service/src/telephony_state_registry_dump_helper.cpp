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
    result.append("TelephonyStateRegistry BindStartTime = ");
    result.append(service->GetBindStartTime());
    result.append("\n");
    result.append("TelephonyStateRegistry BindEndTime = ");
    result.append(service->GetBindEndTime());
    result.append("\n");
    result.append("TelephonyStateRegistry BindSpendTime = ");
    result.append(service->GetBindSpendTime());
    result.append("\n");
    result.append("TelephonyStateRegistry ServiceRunningState = ");
    result.append(std::to_string(service->GetServiceRunningState()));
    result.append("\n");
    for (int32_t i = 0; i < SIM_SLOT_COUNT; i++) {
        if (WhetherHasSimCard(i)) {
            result.append("SlotId = ");
            result.append(std::to_string(i));
            result.append("\n");
            result.append("SimState = ");
            result.append(SimStateTransition(service->GetSimState(i)));
            result.append("\n");
            result.append("CardType = ");
            result.append(CardTypeTransition(service->GetCardType(i)));
            result.append("\n");
            result.append("LockReason = ");
            result.append(LockReasonTransition(service->GetLockReason(i)));
            result.append("\n");
            result.append("CallState = ");
            result.append(CallStateTransition(service->GetCallState(i)));
            result.append("\n");
            result.append("CellularDataConnectionState = ");
            result.append(DataConnectionStateTransition(service->GetCellularDataConnectionState(i)));
            result.append("\n");
            result.append("CellularDataFlow = ");
            result.append(CellularDataFlowTransition(service->GetCellularDataFlow(i)));
            result.append("\n");
            result.append("CellularDataConnectionNetworkType = ");
            result.append(NetworkTypeTransition(service->GetCellularDataConnectionNetworkType(i)));
            result.append("\n");
        }
    }
}

std::string TelephonyStateRegistryDumpHelper::SimStateTransition(int32_t state) const
{
    std::string result = "";
    switch (static_cast<SimState>(state)) {
        case SimState::SIM_STATE_UNKNOWN:
            result = "SIM_STATE_UNKNOWN";
            break;
        case SimState::SIM_STATE_NOT_PRESENT:
            result = "SIM_STATE_NOT_PRESENT";
            break;
        case SimState::SIM_STATE_LOCKED:
            result = "SIM_STATE_LOCKED";
            break;
        case SimState::SIM_STATE_NOT_READY:
            result = "SIM_STATE_NOT_READY";
            break;
        case SimState::SIM_STATE_READY:
            result = "SIM_STATE_READY";
            break;
        case SimState::SIM_STATE_LOADED:
            result = "SIM_STATE_LOADED";
            break;
        default:
            break;
    }
    return result;
}

std::string TelephonyStateRegistryDumpHelper::CallStateTransition(int32_t state) const
{
    std::string result = "CALL_STATUS_IDLE";
    switch (static_cast<TelCallState>(state)) {
        case TelCallState::CALL_STATUS_ACTIVE:
            result = "CALL_STATUS_ACTIVE";
            break;
        case TelCallState::CALL_STATUS_HOLDING:
            result = "CALL_STATUS_HOLDING";
            break;
        case TelCallState::CALL_STATUS_DIALING:
            result = "CALL_STATUS_DIALING";
            break;
        case TelCallState::CALL_STATUS_ALERTING:
            result = "CALL_STATUS_ALERTING";
            break;
        case TelCallState::CALL_STATUS_INCOMING:
            result = "CALL_STATUS_INCOMING";
            break;
        case TelCallState::CALL_STATUS_WAITING:
            result = "CALL_STATUS_WAITING";
            break;
        case TelCallState::CALL_STATUS_DISCONNECTED:
            result = "CALL_STATUS_DISCONNECTED";
            break;
        case TelCallState::CALL_STATUS_DISCONNECTING:
            result = "CALL_STATUS_DISCONNECTING";
            break;
        case TelCallState::CALL_STATUS_IDLE:
            result = "CALL_STATUS_IDLE";
            break;
        default:
            break;
    }
    return result;
}

std::string TelephonyStateRegistryDumpHelper::CardTypeTransition(int32_t type) const
{
    std::string result = "";
    switch (static_cast<CardType>(type)) {
        case CardType::UNKNOWN_CARD:
            result = "UNKNOWN_CARD";
            break;
        case CardType::SINGLE_MODE_SIM_CARD:
            result = "SINGLE_MODE_SIM_CARD";
            break;
        case CardType::SINGLE_MODE_USIM_CARD:
            result = "SINGLE_MODE_USIM_CARD";
            break;
        case CardType::SINGLE_MODE_RUIM_CARD:
            result = "SINGLE_MODE_RUIM_CARD";
            break;
        case CardType::DUAL_MODE_CG_CARD:
            result = "DUAL_MODE_CG_CARD";
            break;
        case CardType::CT_NATIONAL_ROAMING_CARD:
            result = "CT_NATIONAL_ROAMING_CARD";
            break;
        case CardType::CU_DUAL_MODE_CARD:
            result = "CU_DUAL_MODE_CARD";
            break;
        case CardType::DUAL_MODE_TELECOM_LTE_CARD:
            result = "DUAL_MODE_TELECOM_LTE_CARD";
            break;
        case CardType::DUAL_MODE_UG_CARD:
            result = "DUAL_MODE_UG_CARD";
            break;
        case CardType::SINGLE_MODE_ISIM_CARD:
            result = "SINGLE_MODE_ISIM_CARD";
            break;
        default:
            break;
    }
    return result;
}

std::string TelephonyStateRegistryDumpHelper::DataConnectionStateTransition(int32_t state) const
{
    std::string result = "";
    switch (static_cast<DataConnectionStatus>(state)) {
        case DataConnectionStatus::DATA_STATE_DISCONNECTED:
            result = "DATA_STATE_DISCONNECTED";
            break;
        case DataConnectionStatus::DATA_STATE_CONNECTING:
            result = "DATA_STATE_CONNECTING";
            break;
        case DataConnectionStatus::DATA_STATE_CONNECTED:
            result = "DATA_STATE_CONNECTED";
            break;
        case DataConnectionStatus::DATA_STATE_SUSPENDED:
            result = "DATA_STATE_SUSPENDED";
            break;
        default:
            break;
    }
    return result;
}

std::string TelephonyStateRegistryDumpHelper::CellularDataFlowTransition(int32_t flowData) const
{
    std::string result = "";
    switch (static_cast<CellDataFlowType>(flowData)) {
        case CellDataFlowType::DATA_FLOW_TYPE_NONE:
            result = "DATA_FLOW_TYPE_NONE";
            break;
        case CellDataFlowType::DATA_FLOW_TYPE_DOWN:
            result = "DATA_FLOW_TYPE_DOWN";
            break;
        case CellDataFlowType::DATA_FLOW_TYPE_UP:
            result = "DATA_FLOW_TYPE_UP";
            break;
        case CellDataFlowType::DATA_FLOW_TYPE_UP_DOWN:
            result = "DATA_FLOW_TYPE_UP_DOWN";
            break;
        case CellDataFlowType::DATA_FLOW_TYPE_DORMANT:
            result = "DATA_FLOW_TYPE_DORMANT";
            break;
        default:
            break;
    }
    return result;
}

std::string TelephonyStateRegistryDumpHelper::NetworkTypeTransition(int32_t type) const
{
    std::string result = "";
    switch (static_cast<RadioTech>(type)) {
        case RadioTech::RADIO_TECHNOLOGY_UNKNOWN:
            result = "RADIO_TECHNOLOGY_UNKNOWN";
            break;
        case RadioTech::RADIO_TECHNOLOGY_GSM:
            result = "RADIO_TECHNOLOGY_GSM";
            break;
        case RadioTech::RADIO_TECHNOLOGY_1XRTT:
            result = "RADIO_TECHNOLOGY_1XRTT";
            break;
        case RadioTech::RADIO_TECHNOLOGY_WCDMA:
            result = "RADIO_TECHNOLOGY_WCDMA";
            break;
        case RadioTech::RADIO_TECHNOLOGY_HSPA:
            result = "RADIO_TECHNOLOGY_HSPA";
            break;
        case RadioTech::RADIO_TECHNOLOGY_HSPAP:
            result = "RADIO_TECHNOLOGY_HSPAP";
            break;
        case RadioTech::RADIO_TECHNOLOGY_TD_SCDMA:
            result = "RADIO_TECHNOLOGY_TD_SCDMA";
            break;
        case RadioTech::RADIO_TECHNOLOGY_EVDO:
            result = "RADIO_TECHNOLOGY_EVDO";
            break;
        case RadioTech::RADIO_TECHNOLOGY_EHRPD:
            result = "RADIO_TECHNOLOGY_EHRPD";
            break;
        case RadioTech::RADIO_TECHNOLOGY_LTE:
            result = "RADIO_TECHNOLOGY_LTE";
            break;
        case RadioTech::RADIO_TECHNOLOGY_LTE_CA:
            result = "RADIO_TECHNOLOGY_LTE_CA";
            break;
        case RadioTech::RADIO_TECHNOLOGY_IWLAN:
            result = "RADIO_TECHNOLOGY_IWLAN";
            break;
        case RadioTech::RADIO_TECHNOLOGY_NR:
            result = "RADIO_TECHNOLOGY_NR";
            break;
        default:
            break;
    }
    return result;
}

std::string TelephonyStateRegistryDumpHelper::LockReasonTransition(int32_t reason) const
{
    std::string result = "";
    switch (static_cast<LockReason>(reason)) {
        case LockReason::SIM_NONE:
            result = "SIM_NONE";
            break;
        case LockReason::SIM_PIN:
            result = "SIM_PIN";
            break;
        case LockReason::SIM_PUK:
            result = "SIM_PUK";
            break;
        case LockReason::SIM_PN_PIN:
            result = "SIM_PN_PIN";
            break;
        case LockReason::SIM_PN_PUK:
            result = "SIM_PN_PUK";
            break;
        case LockReason::SIM_PU_PIN:
            result = "SIM_PU_PIN";
            break;
        case LockReason::SIM_PU_PUK:
            result = "SIM_PU_PUK";
            break;
        case LockReason::SIM_PP_PIN:
            result = "SIM_PP_PIN";
            break;
        case LockReason::SIM_PP_PUK:
            result = "SIM_PP_PUK";
            break;
        case LockReason::SIM_PC_PIN:
            result = "SIM_PC_PIN";
            break;
        case LockReason::SIM_PC_PUK:
            result = "SIM_PC_PUK";
            break;
        case LockReason::SIM_SIM_PIN:
            result = "SIM_SIM_PIN";
            break;
        case LockReason::SIM_SIM_PUK:
            result = "SIM_SIM_PUK";
            break;
        default:
            break;
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS
