/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "mock_telephony_permission.h"
#include "telephony_permission.h"

namespace OHOS {
namespace Telephony {
std::mutex MockTelephonyPermission::mutex_;
std::shared_ptr<MockTelephonyPermission> MockTelephonyPermission::permission_ = nullptr;

std::shared_ptr<MockTelephonyPermission> MockTelephonyPermission::GetOrCreateMockTelephonyPermission()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (permission_ == nullptr) {
        permission_ = std::make_shared<MockTelephonyPermission>();
    }
    return permission_;
}

void MockTelephonyPermission::ReleaseMockTelephonyPermission()
{
    std::lock_guard<std::mutex> lock(mutex_);
    permission_.reset();
    permission_ = nullptr;
}

bool TelephonyPermission::CheckPermission(const std::string &permissionName)
{
    return MockTelephonyPermission::GetOrCreateMockTelephonyPermission()->CheckPermission(permissionName);
}
} // namespace Telephony
} // namespace OHOS
