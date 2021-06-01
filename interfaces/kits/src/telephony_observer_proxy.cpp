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
#include "telephony_observer_proxy.h"

#include "parcel.h"

#include "string_ex.h"

namespace OHOS {
namespace TelephonyState {
TelephonyObserverProxy::TelephonyObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<TelephonyObserverBroker>(impl)
{}

void TelephonyObserverProxy::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber)
{
    std::string utf8 = Str16ToStr8(phoneNumber);
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    dataParcel.WriteInt32(callState);
    dataParcel.WriteString16(phoneNumber);
    int error = Remote()->SendRequest(ON_CALL_STATE_UPDATED, dataParcel, replyParcel, option);
    HILOG_DEBUG("TelephonyObserverProxy::OnCallStateUpdated end ##error: %{public}d.\n", error);
};

void TelephonyObserverProxy::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    dataParcel.WriteInt32(static_cast<int32_t>(vec.size()));
    for (const auto &v : vec) {
        v->Marshalling(dataParcel);
    }
    int errorCode = Remote()->SendRequest(ON_SIGNAL_INFO_UPDATED, dataParcel, replyParcel, option);
    HILOG_DEBUG("TelephonyObserverProxy::OnSignalInfoUpdated##error: %{public}d.\n", errorCode);
};

void TelephonyObserverProxy::OnNetworkStateUpdated(const sptr<NetworkState> &networkState)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (networkState != nullptr) {
        networkState->Marshalling(dataParcel);
    }
    int errorCode = Remote()->SendRequest(ON_NETWORK_STATE_UPDATED, dataParcel, replyParcel, option);
    HILOG_DEBUG("TelephonyObserverProxy::OnNetworkStateUpdated##error: %{public}d.\n", errorCode);
};
} // namespace TelephonyState
} // namespace OHOS