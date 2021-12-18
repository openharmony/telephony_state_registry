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
        callState = info.callState;
        phoneNumber = info.phoneNumber;
        return *this;
    }
};

struct SignalListContext : EventListener {
    std::vector<sptr<SignalInformation>> signalInfoList;
    SignalListContext &operator=(SignalUpdateInfo &info)
    {
        signalInfoList.swap(info.signalInfoList);
        return *this;
    }
};

struct NetworkStateContext : EventListener {
    sptr<NetworkState> networkState;
    NetworkStateContext &operator=(const NetworkStateUpdateInfo &info)
    {
        networkState = info.networkState;
        return *this;
    }
};

struct SimStateContext : EventListener {
    SimState simState;
    LockReason reason;
    SimStateContext &operator=(const SimStateUpdateInfo &info)
    {
        simState = info.state;
        reason = info.reason;
        return *this;
    }
};

struct CellInfomationContext : EventListener {
    std::vector<sptr<CellInformation>> cellInfoVec;
    CellInfomationContext &operator=(CellInfomationUpdate &info)
    {
        cellInfoVec.swap(info.cellInfoVec);
        return *this;
    }
};
} // namespace Telephony
} // namespace OHOS
#endif // UPDATE_CONTEXTS_H