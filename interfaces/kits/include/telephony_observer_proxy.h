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
#ifndef TELEPHONY_OBSERVER_PROXY_H
#define TELEPHONY_OBSERVER_PROXY_H

#include "iremote_proxy.h"

#include "hilog_wrapper.h"
#include "telephony_observer_broker.h"

namespace OHOS {
namespace TelephonyState {
class TelephonyObserverProxy : public IRemoteProxy<TelephonyObserverBroker> {
public:
    explicit TelephonyObserverProxy(const sptr<IRemoteObject> &impl);
    virtual ~TelephonyObserverProxy() = default;
    void OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber) override;
    void OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec) override;
    void OnNetworkStateUpdated(const sptr<NetworkState> &networkState) override;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, 0, "TelephonyObserverProxy"};

private:
    static inline BrokerDelegator<TelephonyObserverProxy> delegator_;
};
} // namespace TelephonyState
} // namespace OHOS
#endif // TELEPHONY_OBSERVER_PROXY_H