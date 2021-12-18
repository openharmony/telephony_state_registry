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

#ifndef UPDATE_INFOS_H
#define UPDATE_INFOS_H

#include <vector>
#include <memory>
#include <string>

#include "cell_information.h"
#include "signal_information.h"
#include "network_state.h"
#include "refbase.h"
#include "sim_state_type.h"

namespace OHOS {
namespace Telephony {
struct CallStateUpdateInfo {
    int32_t callState;
    std::u16string phoneNumber;
    CallStateUpdateInfo(int32_t callStateParam, std::u16string phoneNumberParam)
        : callState(callStateParam), phoneNumber(phoneNumberParam)
    {}
};

struct SignalUpdateInfo {
    std::vector<sptr<SignalInformation>> signalInfoList;
    SignalUpdateInfo(std::vector<sptr<SignalInformation>> infoList) : signalInfoList(infoList) {}
};

struct NetworkStateUpdateInfo {
    sptr<NetworkState> networkState;
    NetworkStateUpdateInfo(sptr<NetworkState> state) : networkState(state) {}
};

struct SimStateUpdateInfo {
    SimState state;
    LockReason reason;
    SimStateUpdateInfo(SimState simState, LockReason theReason) : state(simState), reason(theReason) {}
};

struct CellInfomationUpdate {
    std::vector<sptr<CellInformation>> cellInfoVec;
    CellInfomationUpdate(const std::vector<sptr<CellInformation>> &cellInfo) : cellInfoVec(cellInfo) {}
};
} // namespace Telephony
} // namespace OHOS
#endif // UPDATE_INFOS_H