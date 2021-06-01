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
#include "telephony_state_registry_stub.h"

#include <utility>

#include "ipc_skeleton.h"

#include "string_ex.h"
#include "telephony_errors.h"

namespace OHOS {
namespace TelephonyState {
TelephonyStateRegistryStub::TelephonyStateRegistryStub()
{
    memberFuncMap_[SIGNAL_INFO] = &TelephonyStateRegistryStub::UpdateSignalInfoInner;
    memberFuncMap_[NET_WORK_STATE] = &TelephonyStateRegistryStub::UpdateNetworkStateInner;
    memberFuncMap_[CALL_STATE] = &TelephonyStateRegistryStub::UpdateCallStateInner;
    memberFuncMap_[CALL_STATE_FOR_ID] = &TelephonyStateRegistryStub::UpdateCallStateForSlotIndexInner;
    memberFuncMap_[ADD_OBSERVER] = &TelephonyStateRegistryStub::RegisterStateChangeInner;
    memberFuncMap_[REMOVE_OBSERVER] = &TelephonyStateRegistryStub::UnregisterStateChangeInner;
}

TelephonyStateRegistryStub::~TelephonyStateRegistryStub()
{
    memberFuncMap_.clear();
}

int32_t TelephonyStateRegistryStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest start##code = %{public}u\n", code);
    std::u16string myDescripter = TelephonyStateRegistryStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    pid_t p = IPCSkeleton::GetCallingPid();
    pid_t p1 = IPCSkeleton::GetCallingUid();
    HILOG_DEBUG("CallingPid = %{public}d, CallingUid = %{public}d, code = %{public}u\n", p, p1, code);
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest end##ret = %{public}d\n", ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateCallStateInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest CALL_STATE\n");
    int32_t callState = data.ReadInt32();
    std::u16string phoneNumber = data.ReadString16();
    int32_t ret = UpdateCallState(callState, phoneNumber);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest CALL_STATE end\n");
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateCallStateForSlotIndexInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest CALL_STATE_FOR_ID\n");
    int32_t subId = data.ReadInt32();
    int32_t phoneId = data.ReadInt32();
    int32_t callState = data.ReadInt32();
    std::u16string incomingNumber = data.ReadString16();
    int32_t ret = UpdateCallStateForSlotIndex(subId, phoneId, callState, incomingNumber);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest CALL_STATE_FOR_ID end\n");
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateSignalInfoInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest SIGNAL_INFO\n");
    int32_t subId = data.ReadInt32();
    int32_t phoneId = data.ReadInt32();

    std::vector<sptr<SignalInformation>> result;
    int32_t size = ReadSignalInfoSize(data);
    SignalInformation::NetworkType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<SignalInformation::NetworkType>(data.ReadInt32());
        switch (type) {
            case SignalInformation::NetworkType::GSM: {
                HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest NetworkType::GSM\n");
                std::unique_ptr<GsmSignalInformation> signal = std::make_unique<GsmSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
            } break;
            case SignalInformation::NetworkType::CDMA: {
                HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest NetworkType::CDMA\n");
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
    int32_t ret = UpdateSignalInfo(subId, phoneId, result);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest SIGNAL_INFO end\n");
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateNetworkStateInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest NET_WORK_STATE\n");
    int32_t ret = TELEPHONY_NO_ERROR;
    int32_t subId = data.ReadInt32();
    int32_t phoneId = data.ReadInt32();
    sptr<NetworkState> result = NetworkState::UnMarshalling(data).get();
    if (result == nullptr) {
        HILOG_DEBUG("TelephonyStateRegistryStub::NotifyNetworkStateUpdatedInner GetNetworkStatus  is null\n");
        ret = TELEPHONY_FAIL;
        return ret;
    }
    ret = UpdateNetworkState(subId, phoneId, result);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest NET_WORK_STATE end\n");
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::RegisterStateChangeInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest ADD_OBSERVER\n");
    int32_t ret = TELEPHONY_NO_ERROR;
    int32_t simId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    bool notifyNow = data.ReadBool();
    std::u16string callingPackage = data.ReadString16();
    sptr<TelephonyObserverBroker> callback = nullptr;
    HILOG_ERROR("TelephonyStateRegistryStub::OnRemoteRequest  ReadData\n");
    ret = ReadData(data, reply, callback);
    bool isCallBack = (callback != nullptr);
    std::string utf8 = Str16ToStr8(callingPackage);
    HILOG_DEBUG("callback = %{public}d, callingPackage = %{public}s\n", isCallBack, utf8.c_str());
    if (ret != TELEPHONY_NO_ERROR) {
        reply.WriteInt32(ret);
        return ret;
    }
    ret = RegisterStateChange(callback, simId, mask, callingPackage, notifyNow);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest ADD_OBSERVER end## ret = %{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UnregisterStateChangeInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest REMOVE_OBSERVER\n");
    int32_t simId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    int32_t ret = UnregisterStateChange(simId, mask);
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest REMOVE_OBSERVER end\n");
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::ReadData(
    MessageParcel &data, MessageParcel &reply, sptr<TelephonyObserverBroker> &callback)
{
    int32_t result = TELEPHONY_NO_ERROR;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOG_ERROR("TelephonyStateRegistryStub::OnRemoteRequest  remote is nullptr.\n");
        result = TELEPHONY_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    callback = iface_cast<TelephonyObserverBroker>(remote);
    if (callback == nullptr) {
        HILOG_ERROR("TelephonyStateRegistryStub::OnRemoteRequest callback is nullptr.\n");
        result = TELEPHONY_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    return result;
}

int32_t TelephonyStateRegistryStub::ReadSignalInfoSize(MessageParcel &data)
{
    int32_t size = data.ReadInt32();
    HILOG_DEBUG("TelephonyStateRegistryStub::OnRemoteRequest SIGNAL_INFO:size=%{public}d\n", size);
    int maxSize = 2;
    if (size > maxSize) {
        size = 0;
        return size;
    }
    return size;
}

int32_t TelephonyStateRegistryStub::RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t simId, uint32_t mask, const std::u16string &package, bool isUpdate)
{
    return RegisterStateChange(telephonyObserver, simId, mask, package, isUpdate, IPCSkeleton::GetCallingPid());
}

int32_t TelephonyStateRegistryStub::UnregisterStateChange(int32_t simId, uint32_t mask)
{
    return UnregisterStateChange(simId, mask, IPCSkeleton::GetCallingPid());
}
} // namespace TelephonyState
} // namespace OHOS