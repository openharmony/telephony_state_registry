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

#include <memory>
#include <string>
#include <vector>

#include "cell_information.h"
#include "network_state.h"
#include "refbase.h"
#include "signal_information.h"
#include "sim_state_type.h"

namespace OHOS {
namespace Telephony {
struct UpdateInfo {
    int32_t slotId_ = 0;
    explicit UpdateInfo(int32_t slotId) : slotId_(slotId) {}
};

struct CallStateUpdateInfo : public UpdateInfo {
    int32_t callState_;
    std::u16string phoneNumber_;
    CallStateUpdateInfo(int32_t slotId, int32_t callStateParam, std::u16string phoneNumberParam)
        : UpdateInfo(slotId), callState_(callStateParam), phoneNumber_(phoneNumberParam)
    {}
};

struct SignalUpdateInfo : public UpdateInfo {
    std::vector<sptr<SignalInformation>> signalInfoList_;
    SignalUpdateInfo(int32_t slotId, std::vector<sptr<SignalInformation>> infoList)
        : UpdateInfo(slotId), signalInfoList_(infoList)
    {}
};

struct NetworkStateUpdateInfo : public UpdateInfo {
    sptr<NetworkState> networkState_;
    NetworkStateUpdateInfo(int32_t slotId, sptr<NetworkState> state) : UpdateInfo(slotId), networkState_(state) {}
};

struct SimStateUpdateInfo : public UpdateInfo {
    CardType type_;
    SimState state_;
    LockReason reason_;
    SimStateUpdateInfo(int32_t slotId, CardType type, SimState simState, LockReason theReason)
        : UpdateInfo(slotId), type_(type), state_(simState), reason_(theReason)
    {}
};

struct CellInfomationUpdate : public UpdateInfo {
    std::vector<sptr<CellInformation>> cellInfoVec_;
    CellInfomationUpdate(int32_t slotId, const std::vector<sptr<CellInformation>> &cellInfo)
        : UpdateInfo(slotId), cellInfoVec_(cellInfo)
    {}
};

struct CellularDataConnectState : public UpdateInfo {
    int32_t dataState_;
    int32_t networkType_;
    CellularDataConnectState(int32_t slotId, int32_t dataState, int32_t networkType)
        : UpdateInfo(slotId), dataState_(dataState), networkType_(networkType)
    {}
};

struct CellularDataFlowUpdate : public UpdateInfo {
    int32_t flowType_;
    CellularDataFlowUpdate(int32_t slotId, int32_t flowType) : UpdateInfo(slotId), flowType_(flowType) {}
};
} // namespace Telephony
} // namespace OHOS
#endif // UPDATE_INFOS_H
