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
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void TelephonyObserver::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber) {}

void TelephonyObserver::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec) {}

void TelephonyObserver::OnNetworkStateUpdated(const sptr<NetworkState> &networkState) {}

void TelephonyObserver::OnCellInfoUpdated(const std::vector<sptr<CellInformation>> &vec) {}

void TelephonyObserver::OnSimStateUpdated(SimState state, LockReason reason) {}

void TelephonyObserver::OnCellularDataConnectStateUpdated(int32_t dataState, int32_t networkType) {}

int32_t TelephonyObserver::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TELEPHONY_LOGI("TelephonyObserver::OnRemoteRequest code = %{public}u......\n", code);
    switch (static_cast<ObserverBrokerCode>(code)) {
        case ObserverBrokerCode::ON_CALL_STATE_UPDATED: {
            int32_t callState = data.ReadInt32();
            std::u16string phoneNumber = data.ReadString16();
            OnCallStateUpdated(callState, phoneNumber);
            break;
        }
        case ObserverBrokerCode::ON_SIGNAL_INFO_UPDATED: {
            std::vector<sptr<SignalInformation>> signalInfos;
            ConvertSignalInfoList(data, signalInfos);
            OnSignalInfoUpdated(signalInfos);
            break;
        }
        case ObserverBrokerCode::ON_CELL_INFO_UPDATED: {
            std::vector<sptr<CellInformation>> cells;
            ConvertCellInfoList(data, cells);
            OnCellInfoUpdated(cells);
            break;
        }
        case ObserverBrokerCode::ON_NETWORK_STATE_UPDATED: {
            sptr<NetworkState> networkState = NetworkState::Unmarshalling(data);
            OnNetworkStateUpdated(networkState);
            break;
        }
        case ObserverBrokerCode::ON_SIM_STATE_UPDATED: {
            SimState simState = static_cast<SimState>(data.ReadInt32());
            LockReason reson = static_cast<LockReason>(data.ReadInt32());
            OnSimStateUpdated(simState, reson);
            break;
        }
        default: {
            return OHOS::UNKNOWN_TRANSACTION;
        }
    }
    return OHOS::NO_ERROR;
}

void TelephonyObserver::ConvertSignalInfoList(MessageParcel &data, std::vector<sptr<SignalInformation>> &result)
{
    int32_t size = data.ReadInt32();
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
                TELEPHONY_LOGI("TelephonyObserver::ConvertSignalInfoList NetworkType::LTE\n");
                std::unique_ptr<LteSignalInformation> signal = std::make_unique<LteSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::WCDMA: {
                TELEPHONY_LOGI("TelephonyObserver::ConvertSignalInfoList NetworkType::Wcdma\n");
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
void TelephonyObserver::ConvertCellInfoList(MessageParcel &data, std::vector<sptr<CellInformation>> &cells)
{
    int32_t size = data.ReadInt32();
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
