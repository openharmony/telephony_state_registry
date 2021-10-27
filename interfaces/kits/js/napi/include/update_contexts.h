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

#include <vector>
#include <string>

#include "refbase.h"
#include "signal_information.h"
#include "network_state.h"
#include "event_listener.h"

namespace OHOS {
namespace Telephony {
struct CallStateContext : EventListener {
    int32_t callState;
    std::u16string phoneNumber;
};

struct SignalListContext : EventListener {
    std::vector<sptr<SignalInformation>> signalInfoList;
};

struct NetworkStateContext : EventListener {
    sptr<NetworkState> networkState;
};

struct SimStateContext : EventListener {
    int32_t simState;
    std::u16string reason;
};
} // namespace Telephony
} // namespace OHOS
#endif // UPDATE_CONTEXTS_H