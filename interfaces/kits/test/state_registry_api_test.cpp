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
#include <iostream>

#include "ipc_skeleton.h"

#include "string_ex.h"
#include "telephony_state_manager.h"

using namespace OHOS;
using namespace TelephonyState;
int main()
{
    TelephonyStateManager manager;
    bool loopFlag = true;
    int32_t subId = 0;
    uint32_t mask = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE;
    int interfaceNum = 0;
    const int32_t exit = 100;
    while (loopFlag) {
        printf(
            "\n**********Unit Test Start**********\n"
            "usage: please input a cmd num:\n"
            "7:AddStateObserver\n"
            "8:RemoveStateObserver\n"
            "100:exit\n"
            "***********************************\n"
            "your choice: ");
        std::cin >> interfaceNum;
        switch (interfaceNum) {
            case ITelephonyStateNotify::ADD_OBSERVER: {
                std::unique_ptr<TelephonyObserver> telephonyObserver = std::make_unique<TelephonyObserver>();
                std::string callingPackage("callingPackage");
                std::u16string str = OHOS::Str8ToStr16(callingPackage);
                bool notifyNow = true;
                manager.AddStateObserver(telephonyObserver.release(), subId, mask, str, notifyNow);
                break;
            }
            case ITelephonyStateNotify::REMOVE_OBSERVER: {
                manager.RemoveStateObserver(subId, mask);
                break;
            }
            case exit: {
                loopFlag = false;
                break;
            }
            default: {
                break;
            }
        }
    }
    printf("telephony_api_test main end.....\n");
    return 0;
}
