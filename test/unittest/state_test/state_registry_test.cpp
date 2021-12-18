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

#include "state_registry_test.h"
#include "telephony_log_wrapper.h"
#include "sim_state_type.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
void StateRegistryTest::SetUpTestCase(void)
{
    // step 3: Set Up Test Case
    printf("SetUpTestCase\n");
}

void StateRegistryTest::TearDownTestCase(void)
{
    // step 3: Tear Down Test Case
    printf("TearDownTestCase\n");
}

void StateRegistryTest::CreateProxy()
{
    if (telephonyStateNotify_ != nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> systemAbilityMgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        printf("StateRegisterService Get ISystemAbilityManager failed.\n");
        return;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    if (remote == nullptr) {
        printf("StateRegisterService Remote service not exists.\n");
        return;
    }
    telephonyStateNotify_ = iface_cast<ITelephonyStateNotify>(remote);
}

void StateRegistryTest::SetUp(void)
{
    // step 3: input testcase setup step
    requestFuncMap_['A'] = &StateRegistryTest::UpdateCallState;
    requestFuncMap_['B'] = &StateRegistryTest::UpdateCallStateForSimId;
    requestFuncMap_['C'] = &StateRegistryTest::UpdateSignalInfo;
    requestFuncMap_['D'] = &StateRegistryTest::UpdateCellularDataConnectState;
    requestFuncMap_['E'] = &StateRegistryTest::UpdateSimState;
    requestFuncMap_['F'] = &StateRegistryTest::UpdateNetworkState;
}

void StateRegistryTest::TearDown(void)
{
    // step 3: input testcase teardown step
}

void StateRegistryTest::UpdateCallState()
{
    CreateProxy();
    if (telephonyStateNotify_ != nullptr) {
        int32_t callState = 16;
        std::string phoneNumber("137xxxxxxxx");
        std::u16string number = Str8ToStr16(phoneNumber);
        telephonyStateNotify_->UpdateCallState(callState, number);
    }
}

void StateRegistryTest::UpdateCallStateForSimId()
{
    CreateProxy();
    if (telephonyStateNotify_ != nullptr) {
        int32_t callState = 16;
        int32_t simId = 1;
        int32_t callId = 0;
        std::string phoneNumber("137xxxxxxxx");
        std::u16string number = Str8ToStr16(phoneNumber);
        telephonyStateNotify_->UpdateCallStateForSimId(simId, callId, callState, number);
    }
}

void StateRegistryTest::UpdateSignalInfo()
{
    CreateProxy();
    if (telephonyStateNotify_ != nullptr) {
        int32_t simId = 1;
        std::vector<sptr<SignalInformation>> vec;
        std::unique_ptr<SignalInformation> signal = std::make_unique<GsmSignalInformation>();
        if (signal == nullptr) {
            TELEPHONY_LOGE("SignalInformation is nullptr\n");
        }
        vec.push_back(signal.release());
        telephonyStateNotify_->UpdateSignalInfo(simId, vec);
    }
}

void StateRegistryTest::UpdateCellularDataConnectState()
{
    CreateProxy();
    if (telephonyStateNotify_ != nullptr) {
        int32_t simId = 1;
        int32_t dataState = 1;
        int32_t networkState = 1;
        telephonyStateNotify_->UpdateCellularDataConnectState(simId, dataState, networkState);
    }
}

void StateRegistryTest::UpdateSimState()
{
    CreateProxy();
    if (telephonyStateNotify_ != nullptr) {
        int32_t simId = 1;
        SimState state = SimState::SIM_STATE_UNKNOWN;
        LockReason reason = LockReason::SIM_NONE;
        telephonyStateNotify_->UpdateSimState(simId, state, reason);
    }
}

void StateRegistryTest::UpdateNetworkState()
{
    CreateProxy();
    if (telephonyStateNotify_ != nullptr) {
        int32_t simId = 1;
        std::unique_ptr<NetworkState> networkState = std::make_unique<NetworkState>();
        if (networkState == nullptr) {
            TELEPHONY_LOGE("NetworkState is nullptr\n");
        }
        telephonyStateNotify_->UpdateNetworkState(simId, networkState.release());
    }
}

HWTEST_F(StateRegistryTest, UpdateCallState_001, TestSize.Level1)
{
    UpdateCallState();
}

HWTEST_F(StateRegistryTest, UpdateCallStateForSimId_001, TestSize.Level1)
{
    UpdateCallStateForSimId();
}

HWTEST_F(StateRegistryTest, UpdateSignalInfo_001, TestSize.Level1)
{
    UpdateSignalInfo();
}

HWTEST_F(StateRegistryTest, UpdateCellularDataConnectState_001, TestSize.Level1)
{
    UpdateCellularDataConnectState();
}

HWTEST_F(StateRegistryTest, UpdateSimState_001, TestSize.Level1)
{
    UpdateSimState();
}

HWTEST_F(StateRegistryTest, UpdateNetworkState_001, TestSize.Level1)
{
    UpdateNetworkState();
}
} // namespace Telephony
} // namespace OHOS