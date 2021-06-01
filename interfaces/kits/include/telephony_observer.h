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
namespace TelephonyState {
class TelephonyObserver : public IRemoteStub<TelephonyObserverBroker> {
public:
    TelephonyObserver() {};
    ~TelephonyObserver() {};
    virtual void OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber) override;
    virtual void OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec) override;
    virtual void OnNetworkStateUpdated(const sptr<NetworkState> &networkState) override;
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    void ConvertSignalInforList(MessageParcel &data, std::vector<sptr<SignalInformation>> &signalInformations);
};
} // namespace TelephonyState
} // namespace OHOS
#endif // TELEPHONY_OBSERVER_H