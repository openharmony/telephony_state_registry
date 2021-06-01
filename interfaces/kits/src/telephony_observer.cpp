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
#include "hilog_wrapper.h"
#include "ipc_types.h"
namespace OHOS {
namespace TelephonyState {
void TelephonyObserver::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber) {}

void TelephonyObserver::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec) {}

void TelephonyObserver::OnNetworkStateUpdated(const sptr<NetworkState> &networkState) {}

int32_t TelephonyObserver::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_INFO("TelephonyObserver::OnRemoteRequest code = %u......\n", code);
    switch (code) {
        case TelephonyState::TelephonyObserverBroker::ON_CALL_STATE_UPDATED: {
            int32_t callState = data.ReadInt32();
            std::u16string phoneNumber = data.ReadString16();
            OnCallStateUpdated(callState, phoneNumber);
            break;
        }
        case TelephonyState::TelephonyObserverBroker::ON_SIGNAL_INFO_UPDATED: {
            std::vector<sptr<SignalInformation>> signalInformations;
            ConvertSignalInforList(data, signalInformations);
            OnSignalInfoUpdated(signalInformations);
            break;
        }
        case TelephonyState::TelephonyObserverBroker::ON_NETWORK_STATE_UPDATED: {
            sptr<NetworkState> networkState = NetworkState::UnMarshalling(data).release();
            OnNetworkStateUpdated(networkState);
            break;
        }
        default: {
            return OHOS::UNKNOWN_TRANSACTION;
        }
    }
    return OHOS::NO_ERROR;
}

void TelephonyObserver::ConvertSignalInforList(MessageParcel &data, std::vector<sptr<SignalInformation>> &result)
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
            } break;
            case SignalInformation::NetworkType::CDMA: {
                std::unique_ptr<CdmaSignalInformation> signal = std::make_unique<CdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
            } break;
            default:
                break;
        }
    }
}
} // namespace TelephonyState
} // namespace OHOS
