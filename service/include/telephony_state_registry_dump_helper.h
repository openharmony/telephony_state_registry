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

#ifndef TELEPHONY_STATE_REGISTRY_DUMP_HELPER_H
#define TELEPHONY_STATE_REGISTRY_DUMP_HELPER_H

#include <vector>
#include <string>

#include "telephony_state_registry_service.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
class TelephonyStateRegistryDumpHelper {
public:
    explicit TelephonyStateRegistryDumpHelper();
    ~TelephonyStateRegistryDumpHelper() = default;
    bool Dump(const std::vector<std::string> &args,
        std::vector<TelephonyStateRegistryRecord> &stateRecords, std::string &result) const;

private:
    bool ShowTelephonyStateRegistryInfo(
        std::vector<TelephonyStateRegistryRecord> &stateRecords, std::string &result) const;
    void ShowTelephonyChangeState(std::string &result) const;
    bool WhetherHasSimCard(const int32_t slotId) const;
};
} // namespace Telephony
} // namespace OHOS

#endif // TELEPHONY_STATE_REGISTRY_DUMP_HELPER_H
