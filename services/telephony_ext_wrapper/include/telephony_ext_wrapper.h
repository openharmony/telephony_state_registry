/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TELEPHONY_EXT_WRAPPER_H
#define TELEPHONY_EXT_WRAPPER_H

#include "nocopyable.h"
#include "singleton.h"
#include "telephony_state_registry_record.h"

namespace OHOS {
namespace Telephony {
class TelephonyExtWrapper final {
DECLARE_DELAYED_REF_SINGLETON(TelephonyExtWrapper);

public:
    DISALLOW_COPY_AND_MOVE(TelephonyExtWrapper);
    void InitTelephonyExtWrapper();

    typedef void (*ON_NETWORK_STATE_UPDATE)(int32_t slotId, TelephonyStateRegistryRecord record,
        sptr<NetworkState> &targetNetworkState, const sptr<NetworkState> &networkState);
    typedef void (*ON_SIGNAL_INFO_UPDATE)(int32_t slotId, TelephonyStateRegistryRecord record,
         std::vector<sptr<SignalInformation>> &targetVec, const std::vector<sptr<SignalInformation>> &vec);
    typedef void (*ON_CELL_INFO_UPDATE)(int32_t slotId, TelephonyStateRegistryRecord record,
        std::vector<sptr<CellInformation>> &targetVec, const std::vector<sptr<CellInformation>> &vec);
    typedef void (*ON_CELLULAR_DATA_CONNECT_STATE_UPDATE)(int32_t slotId, TelephonyStateRegistryRecord record,
        int32_t &networkType);
    typedef void (*SEND_NETWORK_STATE_CHANGED)(int32_t slotId, const sptr<NetworkState> &networkState);
    typedef void (*SEND_SIGNAL_INFO_CHANGED)(int32_t slotId, const std::vector<sptr<SignalInformation>> &vec);

    ON_NETWORK_STATE_UPDATE onNetworkStateUpdated_ = nullptr;
    ON_SIGNAL_INFO_UPDATE onSignalInfoUpdated_ = nullptr;
    ON_CELL_INFO_UPDATE onCellInfoUpdated_ = nullptr;
    ON_CELLULAR_DATA_CONNECT_STATE_UPDATE onCellularDataConnectStateUpdated_ = nullptr;
    SEND_NETWORK_STATE_CHANGED sendNetworkStateChanged_ = nullptr;
    SEND_SIGNAL_INFO_CHANGED sendSignalInfoChanged_ = nullptr;

private:
    void* telephonyExtWrapperHandle_ = nullptr;
};

#define TELEPHONY_EXT_WRAPPER ::OHOS::DelayedRefSingleton<TelephonyExtWrapper>::GetInstance()
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_EXT_WRAPPER_H