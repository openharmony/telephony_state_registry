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

#ifndef FUNCTION_CALLBACK_HANDLER_H
#define FUNCTION_CALLBACK_HANDLER_H

#include "event_handler.h"
#include "event_runner.h"
#include "event_listener.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Telephony {
class FunctionCallbackHandler : public AppExecFwk::EventHandler {
public:
    FunctionCallbackHandler();
    ~FunctionCallbackHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    explicit FunctionCallbackHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    using HandleFuncType = void (FunctionCallbackHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, HandleFuncType> funcMap_;
    void CallbackSignalInfo(const AppExecFwk::InnerEvent::Pointer &event);
    void CallbackCallState(const AppExecFwk::InnerEvent::Pointer &event);
    void CallbackNetworkState(const AppExecFwk::InnerEvent::Pointer &event);
    void CallbackSimState(const AppExecFwk::InnerEvent::Pointer &event);
};
} // namespace Telephony
} // namespace OHOS
#endif // FUNCTION_CALLBACK_HANDLER_H