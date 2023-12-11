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
    TelephonyObserver();
    ~TelephonyObserver();

    /**
     * @brief Called when call state is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param callState Indicates the call state.
     * @param phoneNumber Indicates the phoneNumber.
     */
    void OnCallStateUpdated(
        int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;

    /**
     * @brief Called when signal info is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param vec Indicates the signal information lists.
     */
    void OnSignalInfoUpdated(
        int32_t slotId, const std::vector<sptr<SignalInformation>> &vec) override;

    /**
     * @brief Called when network state is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param networkState Indicates the NetworkState.
     */
    void OnNetworkStateUpdated(
        int32_t slotId, const sptr<NetworkState> &networkState) override;

    /**
     * @brief Called when cell info is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param vec Indicates the cell information list.
     */
    void OnCellInfoUpdated(
        int32_t slotId, const std::vector<sptr<CellInformation>> &vec) override;

    /**
     * @brief Called when sim state is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param type Indicates the type of sim card.
     * @param state Indicates the sim state.
     * @param reason Indicates the sim lock reason.
     */
    void OnSimStateUpdated(
        int32_t slotId, CardType type, SimState state, LockReason reason) override;

    /**
     * @brief Called when cellular data connect state is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param dataState Indicates the cellular data state.
     * @param networkType Indicates the network type.
     */
    void OnCellularDataConnectStateUpdated(
        int32_t slotId, int32_t dataState, int32_t networkType) override;

    /**
     * @brief Called when cellular data flow type is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param dataFlowType Indicates the data flow type.
     */
    void OnCellularDataFlowUpdated(
        int32_t slotId, int32_t dataFlowType) override;

    /**
     * @brief Called when CFU(Call Forwarding Unconditional) indicator updated.
     *
     * @param slotId Indicates the slot identification.
     * @param cfuResult Indicates whether support CFU, true if support, false if
     * not.
     */
    virtual void OnCfuIndicatorUpdated(int32_t slotId, bool cfuResult) override;

    /**
     * @brief Called when the voice mailbox state corresponding to the monitored slotId
     * is updated.
     *
     * @param slotId Indicates the slot identification.
     * @param voiceMailMsgResult Indicates the result of voice mail message,
     * true if success, false if not.
     */
    virtual void OnVoiceMailMsgIndicatorUpdated(int32_t slotId, bool voiceMailMsgResult) override;
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void OnIccAccountUpdated() override;

private:
    using TelephonyObserverFunc = void (TelephonyObserver::*)(MessageParcel &data, MessageParcel &reply);

    void ConvertSignalInfoList(MessageParcel &data, std::vector<sptr<SignalInformation>> &signalInfos);
    void ConvertLteNrSignalInfoList(
        MessageParcel &data, std::vector<sptr<SignalInformation>> &signalInfos, SignalInformation::NetworkType type);
    void ConvertCellInfoList(MessageParcel &data, std::vector<sptr<CellInformation>> &cells);
    void OnCallStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnSignalInfoUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnNetworkStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCellInfoUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnSimStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCellularDataConnectStateUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCellularDataFlowUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnCfuIndicatorUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnVoiceMailMsgIndicatorUpdatedInner(MessageParcel &data, MessageParcel &reply);
    void OnIccAccountUpdatedInner(MessageParcel &data, MessageParcel &reply);
    static constexpr int32_t CELL_NUM_MAX = 100;
    static constexpr int32_t SIGNAL_NUM_MAX = 100;
    std::map<uint32_t, TelephonyObserverFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_OBSERVER_H
