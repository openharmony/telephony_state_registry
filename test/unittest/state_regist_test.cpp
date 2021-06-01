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
#include <securec.h>

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "system_ability_definition.h"

#include "string_ex.h"

#include "i_telephony_state_notify.h"
#include "telephony_observer.h"

namespace OHOS {
namespace TelephonyState {
void PrintfHint()
{
    printf(
        "\n**********Unit Test Start**********\n"
        "usage: please input a cmd num:\n"
        "1:CELLULAR_DATA\n"
        "2:SIGNAL_INFO\n"
        "3:NET_WORK_TYPE\n"
        "4:NET_WORK_STATE\n"
        "5:CALL_STATE\n"
        "6:CALL_STATE_FOR_ID\n"
        "7:REGIST_BROAD_CAST\n"
        "8:SEND_BROAD_CAST\n"
        "9:UNREGIST_BROAD_CAST\n"
        "100:exit\n"
        "***********************************\n"
        "your choice: ");
}

int32_t NotifyCallStateUpdated(sptr<ITelephonyStateNotify> telephonyService)
{
    int32_t callState = 16;
    std::string phoneNumber("13714532421");
    std::u16string number = Str8ToStr16(phoneNumber);
    return telephonyService->UpdateCallState(callState, number);
}

int32_t UpdateCallStateForSlotIndex(sptr<ITelephonyStateNotify> telephonyService, int32_t subId, int32_t phoneId)
{
    std::string incomingNumber("13714532420");
    int32_t callState = 3;
    std::u16string number = Str8ToStr16(incomingNumber);
    return telephonyService->UpdateCallStateForSlotIndex(subId, phoneId, callState, number);
}

int32_t NotifySignalInfoUpdated(sptr<ITelephonyStateNotify> telephonyService, int32_t subId, int32_t phoneId)
{
    std::vector<sptr<SignalInformation>> vec;
    return telephonyService->UpdateSignalInfo(subId, phoneId, vec);
}

void Looper(sptr<ITelephonyStateNotify> telephonyService)
{
    int interfaceNum = 0;
    bool loopFlag = true;
    int32_t subId = 0;
    int32_t phoneId = 0;
    const int32_t exit = 100;
    while (loopFlag) {
        PrintfHint();
        std::cin >> interfaceNum;
        switch (interfaceNum) {
            case ITelephonyStateNotify::SIGNAL_INFO: {
                NotifySignalInfoUpdated(telephonyService, subId, phoneId);
                break;
            }
            case ITelephonyStateNotify::NET_WORK_STATE: {
                break;
            }
            case ITelephonyStateNotify::CALL_STATE: {
                NotifyCallStateUpdated(telephonyService);
                break;
            }
            case ITelephonyStateNotify::CALL_STATE_FOR_ID: {
                UpdateCallStateForSlotIndex(telephonyService, subId, phoneId);
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
}
} // namespace TelephonyState
} // namespace OHOS

using namespace OHOS;
int main()
{
    int32_t result = 0;
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        printf("TelephonyStateRegistryService Get ISystemAbilityManager failed.\n");
        result = -1;
        return result;
    }

    printf("TelephonyStateRegistryService started...................\n");
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    if (remote) {
        printf("TelephonyStateRegistryService Remote service exists---- \n");
        sptr<TelephonyState::ITelephonyStateNotify> telephonyService =
            iface_cast<TelephonyState::ITelephonyStateNotify>(remote);

        if (telephonyService == nullptr) {
            printf("TelephonyStateRegistryService telephonyService nullptr---- \n");
            result = -1;
            return result;
        }

        TelephonyState::Looper(telephonyService);
    } else {
        printf("TelephonyStateRegistryService Remote service not exists \n");
        result = -1;
        return result;
    }
    return result;
}
