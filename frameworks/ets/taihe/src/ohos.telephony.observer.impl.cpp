/*
 * Copyright (C) 2025-2025 Huawei Device Co., Ltd.
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

#include "ohos.telephony.observer.proj.hpp"
#include "ohos.telephony.observer.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

using namespace taihe;
namespace {
//To be implemented.

void onCallStateChange(::taihe::callback_view<void(::ohos::telephony::observer::CallStateInfo const&)> callback)
{
    return;
}

void onCallStateChange2(::ohos::telephony::observer::ObserverOptions const& options,
    ::taihe::callback_view<void(::ohos::telephony::observer::CallStateInfo const&)> callback)
{
    return;
}

void offCallStateChange(::taihe::optional_view<::taihe::callback<void(
    ::ohos::telephony::observer::CallStateInfo const&)>> callback)
{
    return;
}
} // namespace

// Since these macros are auto-generate, lint will cause false positive.
//NOLINTBEGIN
TH_EXPORT_CPP_API_onCallStateChange(onCallStateChange);
TH_EXPORT_CPP_API_onCallStateChange2(onCallStateChange2);
TH_EXPORT_CPP_API_offCallStateChange(offCallStateChange);
//NOLINTEND