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

#ifndef TELEPHONY_CALLBACK_EVENT_ID_H
#define TELEPHONY_CALLBACK_EVENT_ID_H

namespace OHOS {
namespace Telephony {
enum class TelephonyCallbackEventId : uint32_t {
    EVENT_REMOVE_ONCE = 0,
    EVENT_ON_CALL_STATE_UPDATE = 1,
    EVENT_ON_SIGNAL_INFO_UPDATE = 2,
    EVENT_ON_NETWORK_STATE_UPDATE = 3,
    EVENT_ON_SIM_STATE_UPDATE = 4,
    EVENT_ON_CELL_INFOMATION_UPDATE = 5,
    EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE = 6,
    EVENT_ON_CELLULAR_DATA_FLOW_UPDATE = 7,
};

template<typename EnumClass>
auto ToUint32t(EnumClass const value) -> typename std::underlying_type<EnumClass>::type
{
    return static_cast<typename std::underlying_type<EnumClass>::type>(value);
}
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_CALLBACK_EVENT_ID_H