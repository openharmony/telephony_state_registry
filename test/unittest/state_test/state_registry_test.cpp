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
#include "telephony_state_registry_client.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
void StateRegistryTest::SetUpTestCase(void)
{
    // step 3: Set Up Test Case
}

void StateRegistryTest::TearDownTestCase(void)
{
    // step 3: Tear Down Test Case
}

void StateRegistryTest::SetUp(void)
{
    // step 3: input testcase setup step
    requestFuncMap_['A'] = &StateRegistryTest::UpdateCallState;
    requestFuncMap_['B'] = &StateRegistryTest::UpdateCallStateForSlotId;
    requestFuncMap_['C'] = &StateRegistryTest::UpdateSignalInfo;
    requestFuncMap_['D'] = &StateRegistryTest::UpdateCellularDataConnectState;
    requestFuncMap_['E'] = &StateRegistryTest::UpdateSimState;
    requestFuncMap_['F'] = &StateRegistryTest::UpdateNetworkState;
    requestFuncMap_['G'] = &StateRegistryTest::UpdateCellularDataFlow;
}

void StateRegistryTest::TearDown(void)
{
    // step 3: input testcase teardown step
}

void StateRegistryTest::UpdateCallState()
{
    int32_t callState = 16;
    int32_t slotId = 0;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateCallState(slotId, callState, number);
}

void StateRegistryTest::UpdateCallStateForSlotId()
{
    int32_t callState = 16;
    int32_t slotId = 0;
    int32_t callId = 0;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateCallStateForSlotId(slotId, callId, callState, number);
}

void StateRegistryTest::UpdateSignalInfo()
{
    int32_t slotId = 0;
    std::vector<sptr<SignalInformation>> vec;
    std::unique_ptr<SignalInformation> signal = std::make_unique<GsmSignalInformation>();
    if (signal == nullptr) {
        TELEPHONY_LOGE("SignalInformation is nullptr\n");
    }
    vec.push_back(signal.release());
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateSignalInfo(slotId, vec);
}

void StateRegistryTest::UpdateCellularDataConnectState()
{
    int32_t slotId = 0;
    int32_t dataState = 1;
    int32_t networkState = 1;
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateCellularDataConnectState(slotId, dataState, networkState);
}

void StateRegistryTest::UpdateCellularDataFlow()
{
    int32_t slotId = 0;
    int32_t dataFlowType = 0;
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateCellularDataFlow(slotId, dataFlowType);
}

void StateRegistryTest::UpdateSimState()
{
    int32_t slotId = 0;
    CardType type = CardType::UNKNOWN_CARD;
    SimState state = SimState::SIM_STATE_UNKNOWN;
    LockReason reason = LockReason::SIM_NONE;
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateSimState(slotId, type, state, reason);
}

void StateRegistryTest::UpdateNetworkState()
{
    int32_t slotId = 0;
    std::unique_ptr<NetworkState> networkState = std::make_unique<NetworkState>();
    if (networkState == nullptr) {
        TELEPHONY_LOGE("NetworkState is nullptr\n");
    }
    DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().
        UpdateNetworkState(slotId, networkState.release());
}

HWTEST_F(StateRegistryTest, StateRegistry_001, TestSize.Level0)
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TELEPHONY_LOGE("StateRegistryService Get ISystemAbilityManager failed.");
        return;
    }
    auto remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    if (remote == nullptr) {
        TELEPHONY_LOGE("StateRegistryService Remote service not exists.");
        return;
    }
    auto stateRegistryService = iface_cast<ITelephonyStateNotify>(remote);
    TELEPHONY_LOGI("HWTEST_F StateRegistry_001");
}

HWTEST_F(StateRegistryTest, UpdateCallState_001, TestSize.Level1)
{
    UpdateCallState();
}

HWTEST_F(StateRegistryTest, UpdateCallStateForSlotId_001, TestSize.Level1)
{
    UpdateCallStateForSlotId();
}

HWTEST_F(StateRegistryTest, UpdateSignalInfo_001, TestSize.Level1)
{
    UpdateSignalInfo();
}

HWTEST_F(StateRegistryTest, UpdateCellularDataConnectState_001, TestSize.Level1)
{
    UpdateCellularDataConnectState();
}

HWTEST_F(StateRegistryTest, UpdateCellularDataFlow_001, TestSize.Level1)
{
    UpdateCellularDataFlow();
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