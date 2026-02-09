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

#include "telephony_errors.h"
#include "telephony_observer_proxy.h"

#include "parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace Telephony {
TelephonyObserverProxy::TelephonyObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<TelephonyObserverBroker>(impl)
{}

int32_t TelephonyObserverProxy::SendRequest(
    int32_t msgId, MessageParcel &dataParcel, MessageParcel &replyParcel, MessageOption &option)
{
    auto remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyObserverProxy remote is nullptr!, msgId: %{public}d", msgId);
        return TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL;
    }
    return remote->SendRequest(msgId, dataParcel, replyParcel, option);
}

void TelephonyObserverProxy::OnCallStateUpdated(
    int32_t slotId, int32_t callState, const std::u16string &phoneNumber)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCallStateUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(callState);
    dataParcel.WriteString16(phoneNumber);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CALL_STATE_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCallStateUpdated end ##error: %{public}d", code);
};

void TelephonyObserverProxy::OnCallStateUpdatedEx(
    int32_t slotId, int32_t callStateEx)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCallStateUpdatedEx WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(callStateEx);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CALL_STATE_EX_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCallStateUpdatedEx end ##error: %{public}d", code);
};

void TelephonyObserverProxy::OnSimStateUpdated(
    int32_t slotId, CardType type, SimState state, LockReason reason)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSimStateUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(static_cast<int32_t>(type));
    dataParcel.WriteInt32(static_cast<int32_t>(state));
    dataParcel.WriteInt32(static_cast<int32_t>(reason));
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_SIM_STATE_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGI("TelephonyObserverProxy::OnSimStateUpdated end ##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnSignalInfoUpdated(
    int32_t slotId, const std::vector<sptr<SignalInformation>> &vec)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSignalInfoUpdated WriteInterfaceToken failed!");
        return;
    }
    int32_t size = static_cast<int32_t>(vec.size());
    if (size < 0 || size > SignalInformation::MAX_SIGNAL_NUM) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSignalInfoUpdated size error!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(size);
    for (const auto &v : vec) {
        v->Marshalling(dataParcel);
    }
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_SIGNAL_INFO_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnSignalInfoUpdated##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnCellInfoUpdated(
    int32_t slotId, const std::vector<sptr<CellInformation>> &vec)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCellInfoUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    int32_t size = static_cast<int32_t>(vec.size());
    if (size <= 0) {
        TELEPHONY_LOGE("Cellinformation array length is less than or equal to 0!");
        return;
    }
    if (size > CellInformation::MAX_CELL_NUM) {
        TELEPHONY_LOGE("Cellinformation array length is greater than MAX_CELL_NUM!");
        return;
    }
    if (!dataParcel.WriteInt32(size)) {
        TELEPHONY_LOGE("Failed to write Cellinformation array size!");
        return;
    }
    for (const auto &v : vec) {
        v->Marshalling(dataParcel);
    }
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CELL_INFO_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCellInfoUpdated##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnNetworkStateUpdated(
    int32_t slotId, const sptr<NetworkState> &networkState)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnNetworkStateUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    if (networkState != nullptr) {
        networkState->Marshalling(dataParcel);
    }
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_NETWORK_STATE_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnNetworkStateUpdated##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnCellularDataConnectStateUpdated(
    int32_t slotId, int32_t dataState, int32_t networkType)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCellularDataConnectStateUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(dataState);
    dataParcel.WriteInt32(networkType);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CELLULAR_DATA_CONNECT_STATE_UPDATED),
        dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCellularDataConnectStateUpdated##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnCellularDataFlowUpdated(
    int32_t slotId, int32_t dataFlowType)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC | MessageOption::TF_ASYNC_WAKEUP_LATER);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCellularDataFlowUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(dataFlowType);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CELLULAR_DATA_FLOW_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCellularDataFlow##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnCfuIndicatorUpdated(int32_t slotId, bool cfuResult)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCfuIndicatorUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteBool(cfuResult);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CFU_INDICATOR_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGI("TelephonyObserverProxy::OnCfuIndicatorUpdated##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnVoiceMailMsgIndicatorUpdated(int32_t slotId, bool voiceMailMsgResult)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnVoiceMailMsgIndicatorUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteBool(voiceMailMsgResult);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_VOICE_MAIL_MSG_INDICATOR_UPDATED),
        dataParcel, replyParcel, option);
    TELEPHONY_LOGI("TelephonyObserverProxy::OnVoiceMailMsgIndicatorUpdated##error: %{public}d.", code);
}

void TelephonyObserverProxy::OnIccAccountUpdated()
{
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("WriteInterfaceToken failed!");
        return;
    }
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    MessageParcel replyParcel;
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_ICC_ACCOUNT_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGI("result code: %{public}d.", code);
}

void TelephonyObserverProxy::OnCCallStateUpdated(
    int32_t slotId, int32_t callState, const std::u16string &phoneNumber)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCCallStateUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteInt32(callState);
    dataParcel.WriteString16(phoneNumber);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_CCALL_STATE_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCCallStateUpdated end ##error: %{public}d", code);
};

void TelephonyObserverProxy::OnSimActiveStateUpdated(int32_t slotId, bool enable)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSimActiveStateUpdated WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(slotId);
    dataParcel.WriteBool(enable);
    auto code = SendRequest(
        static_cast<int32_t>(ObserverBrokerCode::ON_SIM_ACTIVE_STATE_UPDATED), dataParcel, replyParcel, option);
    TELEPHONY_LOGI("TelephonyObserverProxy::OnSimActiveStateUpdated##error: %{public}d.", code);
}
} // namespace Telephony
} // namespace OHOS
