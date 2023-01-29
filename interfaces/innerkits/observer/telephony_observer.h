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

#ifndef TELEPHONY_OBSERVER_H
#define TELEPHONY_OBSERVER_H

#include <cstdint>
#include <string>
#include <vector>

#include "iremote_stub.h"

#include "telephony_observer_broker.h"

namespace OHOS {
namespace Telephony {
class TelephonyObserver : public IRemoteStub<TelephonyObserverBroker> {
public:
    TelephonyObserver() {}
    ~TelephonyObserver() {}
    void OnCallStateUpdated(
        int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;
    void OnSignalInfoUpdated(
        int32_t slotId, const std::vector<sptr<SignalInformation>> &vec) override;
    void OnNetworkStateUpdated(
        int32_t slotId, const sptr<NetworkState> &networkState) override;
    void OnCellInfoUpdated(
        int32_t slotId, const std::vector<sptr<CellInformation>> &vec) override;
    void OnSimStateUpdated(
        int32_t slotId, CardType type, SimState state, LockReason reason) override;
    void OnCellularDataConnectStateUpdated(
        int32_t slotId, int32_t dataState, int32_t networkType) override;
    void OnCellularDataFlowUpdated(
        int32_t slotId, int32_t dataFlowType) override;
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    void ConvertSignalInfoList(MessageParcel &data, std::vector<sptr<SignalInformation>> &signalInfos);
    void ConvertCellInfoList(MessageParcel &data, std::vector<sptr<CellInformation>> &cells);
    void OnCallStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnSignalInfoUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnNetworkStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCellInfoUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnSimStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCellularDataConnectStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCellularDataFlowUpdatedInner(MessageParcel &data, MessageParcel &reply);
    static constexpr int32_t CELL_NUM_MAX = 100;
    static constexpr int32_t SIGNAL_NUM_MAX = 100;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_OBSERVER_H
