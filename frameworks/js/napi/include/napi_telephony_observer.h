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

#ifndef NAPI_TELEPHONY_OBSERVER_H
#define NAPI_TELEPHONY_OBSERVER_H

#include <memory>
#include <string>
#include <vector>

#include "signal_information.h"
#include "telephony_observer.h"

namespace OHOS {
namespace Telephony {
class NapiTelephonyObserver : public TelephonyObserver {
public:
    void OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;
    void OnSignalInfoUpdated(int32_t slotId, const std::vector<sptr<SignalInformation>> &vec) override;
    void OnNetworkStateUpdated(int32_t slotId, const sptr<NetworkState> &networkState) override;
    void OnSimStateUpdated(int32_t slotId, CardType type, SimState state, LockReason reason) override;
    void OnCellInfoUpdated(int32_t slotId, const std::vector<sptr<CellInformation>> &vec) override;
    void OnCellularDataConnectStateUpdated(int32_t slotId, int32_t dataState, int32_t networkType) override;
    void OnCellularDataFlowUpdated(int32_t slotId, int32_t dataFlowType) override;
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_TELEPHONY_OBSERVER_H