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

#ifndef UPDATE_CONTEXTS_H
#define UPDATE_CONTEXTS_H

#include <string>
#include <vector>

#include "event_listener.h"
#include "network_state.h"
#include "refbase.h"
#include "signal_information.h"
#include "update_infos.h"

namespace OHOS {
namespace Telephony {
struct CallStateContext : EventListener {
    int32_t callState;
    std::u16string phoneNumber;
    CallStateContext &operator=(const CallStateUpdateInfo &info)
    {
        callState = info.callState_;
        phoneNumber = info.phoneNumber_;
        return *this;
    }
};

struct SignalListContext : EventListener {
    std::vector<sptr<SignalInformation>> signalInfoList;
    SignalListContext &operator=(SignalUpdateInfo &info)
    {
        signalInfoList = info.signalInfoList_;
        return *this;
    }
};

struct NetworkStateContext : EventListener {
    sptr<NetworkState> networkState;
    NetworkStateContext &operator=(const NetworkStateUpdateInfo &info)
    {
        networkState = info.networkState_;
        return *this;
    }
};

struct SimStateContext : EventListener {
    CardType cardType;
    SimState simState;
    LockReason reason;
    SimStateContext &operator=(const SimStateUpdateInfo &info)
    {
        cardType = info.type_;
        simState = info.state_;
        reason = info.reason_;
        return *this;
    }
};

struct CellInfomationContext : EventListener {
    std::vector<sptr<CellInformation>> cellInfoVec;
    CellInfomationContext &operator=(CellInfomationUpdate &info)
    {
        cellInfoVec = info.cellInfoVec_;
        return *this;
    }
};

struct CellularDataConnectStateContext : EventListener {
    int32_t dataState;
    int32_t networkType;
    CellularDataConnectStateContext &operator=(const CellularDataConnectState &info)
    {
        dataState = info.dataState_;
        networkType = info.networkType_;
        return *this;
    }
};

struct CellularDataFlowContext : EventListener {
    int32_t flowType_;
    CellularDataFlowContext &operator=(const CellularDataFlowUpdate &info)
    {
        flowType_ = info.flowType_;
        return *this;
    }
};
} // namespace Telephony
} // namespace OHOS
#endif // UPDATE_CONTEXTS_H