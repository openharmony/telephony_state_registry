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

#ifndef TELEPHONY_STATE_REGISTRY_STUB_H
#define TELEPHONY_STATE_REGISTRY_STUB_H

#include <map>

#include "iremote_stub.h"

#include "telephony_log_wrapper.h"
#include "i_telephony_state_notify.h"
#include "state_registry_ipc_interface_code.h"

namespace OHOS {
namespace Telephony {
class TelephonyStateRegistryStub : public IRemoteStub<ITelephonyStateNotify> {
public:
    TelephonyStateRegistryStub();
    ~TelephonyStateRegistryStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    virtual int32_t RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver, int32_t slotId,
        uint32_t mask, const std::string &bundleName, bool notifyNow, pid_t pid, int32_t uid, int32_t tokenId) = 0;

    virtual int32_t UnregisterStateChange(int32_t slotId, uint32_t mask, int32_t tokenId, pid_t pid) = 0;

private:
    int32_t ReadData(MessageParcel &data, MessageParcel &reply, sptr<TelephonyObserverBroker> &callback);
    int32_t RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
        int32_t slotId, uint32_t mask, bool isUpdate) override;
    int32_t UnregisterStateChange(int32_t slotId, uint32_t mask) override;
    void parseSignalInfos(
        MessageParcel &data, const int32_t size, std::vector<sptr<SignalInformation>> &result);
    void ParseLteNrSignalInfos(
        MessageParcel &data, std::vector<sptr<SignalInformation>> &result, SignalInformation::NetworkType type);

private:
    using TelephonyStateFunc = int32_t (TelephonyStateRegistryStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t OnUpdateCellInfo(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateCallState(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateCallStateForSlotId(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateSignalInfo(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateNetworkState(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateSimState(MessageParcel &data, MessageParcel &reply);
    int32_t OnRegisterStateChange(MessageParcel &data, MessageParcel &reply);
    int32_t OnUnregisterStateChange(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateCellularDataConnectState(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateCellularDataFlow(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateCfuIndicator(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateVoiceMailMsgIndicator(MessageParcel &data, MessageParcel &reply);
    int32_t OnIccAccountUpdated(MessageParcel &data, MessageParcel &reply);

private:
    std::map<StateNotifyInterfaceCode, TelephonyStateFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_STATE_REGISTRY_STUB_H
