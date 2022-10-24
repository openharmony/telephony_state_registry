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

#include "telephony_observer.h"

#include "telephony_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void TelephonyObserver::OnCallStateUpdated(
    int32_t slotId, int32_t callState, const std::u16string &phoneNumber) {}

void TelephonyObserver::OnSignalInfoUpdated(
    int32_t slotId, const std::vector<sptr<SignalInformation>> &vec) {}

void TelephonyObserver::OnNetworkStateUpdated(
    int32_t slotId, const sptr<NetworkState> &networkState) {}

void TelephonyObserver::OnCellInfoUpdated(
    int32_t slotId, const std::vector<sptr<CellInformation>> &vec) {}

void TelephonyObserver::OnSimStateUpdated(
    int32_t slotId, CardType type, SimState state, LockReason reason) {}

void TelephonyObserver::OnCellularDataConnectStateUpdated(
    int32_t slotId, int32_t dataState, int32_t networkType) {}

void TelephonyObserver::OnCellularDataFlowUpdated(
    int32_t slotId, int32_t dataFlowType) {}

int32_t TelephonyObserver::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TELEPHONY_LOGI("TelephonyObserver::OnRemoteRequest code = %{public}u......\n", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TELEPHONY_LOGE("TelephonyObserver::OnRemoteRequest verify token failed!");
        return TELEPHONY_ERR_DESCRIPTOR_MISMATCH;
    }
    switch (static_cast<ObserverBrokerCode>(code)) {
        case ObserverBrokerCode::ON_CALL_STATE_UPDATED: {
            OnCallStateUpdatedInner(data, reply);
            break;
        }
        case ObserverBrokerCode::ON_SIGNAL_INFO_UPDATED: {
            OnSignalInfoUpdatedInner(data, reply);
            break;
        }
        case ObserverBrokerCode::ON_CELL_INFO_UPDATED: {
            OnCellInfoUpdatedInner(data, reply);
            break;
        }
        case ObserverBrokerCode::ON_NETWORK_STATE_UPDATED: {
            OnNetworkStateUpdatedInner(data, reply);
            break;
        }
        case ObserverBrokerCode::ON_SIM_STATE_UPDATED: {
            OnSimStateUpdatedInner(data, reply);
            break;
        }
        case ObserverBrokerCode::ON_CELLULAR_DATA_CONNECT_STATE_UPDATED: {
            OnCellularDataConnectStateUpdatedInner(data, reply);
            break;
        }
        case ObserverBrokerCode::ON_CELLULAR_DATA_FLOW_UPDATED: {
            OnCellularDataFlowUpdatedInner(data, reply);
            break;
        }
        default: {
            return OHOS::UNKNOWN_TRANSACTION;
        }
    }
    return OHOS::NO_ERROR;
}

void TelephonyObserver::OnCallStateUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t callState = data.ReadInt32();
    std::u16string phoneNumber = data.ReadString16();
    OnCallStateUpdated(slotId, callState, phoneNumber);
}

void TelephonyObserver::OnSignalInfoUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    std::vector<sptr<SignalInformation>> signalInfos;
    ConvertSignalInfoList(data, signalInfos);
    OnSignalInfoUpdated(slotId, signalInfos);
}

void TelephonyObserver::OnNetworkStateUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    sptr<NetworkState> networkState = NetworkState::Unmarshalling(data);
    OnNetworkStateUpdated(slotId, networkState);
}

void TelephonyObserver::OnCellInfoUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    std::vector<sptr<CellInformation>> cells;
    ConvertCellInfoList(data, cells);
    OnCellInfoUpdated(slotId, cells);
}

void TelephonyObserver::OnSimStateUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    CardType type = static_cast<CardType>(data.ReadInt32());
    SimState simState = static_cast<SimState>(data.ReadInt32());
    LockReason reson = static_cast<LockReason>(data.ReadInt32());
    OnSimStateUpdated(slotId, type, simState, reson);
}

void TelephonyObserver::OnCellularDataConnectStateUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t dataState = data.ReadInt32();
    int32_t networkType = data.ReadInt32();
    OnCellularDataConnectStateUpdated(slotId, dataState, networkType);
}

void TelephonyObserver::OnCellularDataFlowUpdatedInner(
    MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t flowType = data.ReadInt32();
    OnCellularDataFlowUpdated(slotId, flowType);
}

void TelephonyObserver::ConvertSignalInfoList(
    MessageParcel &data, std::vector<sptr<SignalInformation>> &result)
{
    int32_t size = data.ReadInt32();
    size = (size > SIGNAL_NUM_MAX) ? SIGNAL_NUM_MAX : size;
    SignalInformation::NetworkType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<SignalInformation::NetworkType>(data.ReadInt32());
        switch (type) {
            case SignalInformation::NetworkType::GSM: {
                std::unique_ptr<GsmSignalInformation> signal = std::make_unique<GsmSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::CDMA: {
                std::unique_ptr<CdmaSignalInformation> signal = std::make_unique<CdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::LTE: {
                std::unique_ptr<LteSignalInformation> signal = std::make_unique<LteSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::WCDMA: {
                std::unique_ptr<WcdmaSignalInformation> signal = std::make_unique<WcdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            default:
                break;
        }
    }
}

void TelephonyObserver::ConvertCellInfoList(
    MessageParcel &data, std::vector<sptr<CellInformation>> &cells)
{
    int32_t size = data.ReadInt32();
    size = (size > CELL_NUM_MAX) ? CELL_NUM_MAX : size;
    CellInformation::CellType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<CellInformation::CellType>(data.ReadInt32());
        switch (type) {
            case CellInformation::CellType::CELL_TYPE_GSM: {
                std::unique_ptr<GsmCellInformation> cell = std::make_unique<GsmCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            case CellInformation::CellType::CELL_TYPE_LTE: {
                std::unique_ptr<LteCellInformation> cell = std::make_unique<LteCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            default:
                break;
        }
    }
}
} // namespace Telephony
} // namespace OHOS
