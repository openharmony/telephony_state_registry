/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "state_registry_test.h"

#include "core_service_client.h"
#include "sim_state_type.h"
#include "telephony_ext_wrapper.h"
#include "telephony_log_wrapper.h"
#include "telephony_observer_client.h"
#include "telephony_observer_proxy.h"
#include "telephony_state_manager.h"
#include "telephony_state_registry_client.h"
#include "telephony_state_registry_service.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
sptr<StateRegistryObserver> StateRegistryTest::telephonyObserver0_ = nullptr;
sptr<StateRegistryObserver> StateRegistryTest::telephonyObserver1_ = nullptr;
static constexpr int32_t SIM_SLOT_ID_1 = DEFAULT_SIM_SLOT_ID + 1;
static constexpr int32_t CALL_STATUS_ACTIVE = 0;
static constexpr int32_t SINGLE_MODE_SIM_CARD = 10;
static constexpr int32_t SIM_STATE_NOT_PRESENT = 1;
static constexpr int32_t SIM_PIN = 1;
static constexpr int32_t DATA_STATE_CONNECTING = 1;
static constexpr int32_t NETWORK_TYPE_GSM = 1;
static constexpr int32_t DATA_FLOW_TYPE_DOWN = 1;

void StateRegistryTest::SetUpTestCase(void)
{
    ASSERT_TRUE(CoreServiceClient::GetInstance().GetProxy() != nullptr);
    InitTelephonyObserver();
}

void StateRegistryTest::TearDownTestCase(void)
{
    DisableTelephonyObserver();
}

void StateRegistryTest::InitTelephonyObserver()
{
    TELEPHONY_LOGI("Init telephony observer");
    AccessToken token;
    if (!telephonyObserver0_) {
        telephonyObserver0_ = std::make_unique<StateRegistryObserver>().release();
    }
    auto res =
        Telephony::TelephonyObserverClient::GetInstance().AddStateObserver(telephonyObserver0_, DEFAULT_SIM_SLOT_ID,
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR |
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR,
            true);
    TELEPHONY_LOGI("StateRegistryTest init telephony observer0 ret:%{public}d", res);
    ASSERT_TRUE(res == TELEPHONY_SUCCESS);
    if (!telephonyObserver1_) {
        telephonyObserver1_ = std::make_unique<StateRegistryObserver>().release();
    }
    res = Telephony::TelephonyObserverClient::GetInstance().AddStateObserver(telephonyObserver1_, SIM_SLOT_ID_1,
        Telephony::TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR |
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR,
        true);
    TELEPHONY_LOGI("StateRegistryTest init telephony observer1 ret:%{public}d", res);
    ASSERT_TRUE(res == TELEPHONY_SUCCESS);
}

void StateRegistryTest::DisableTelephonyObserver()
{
    TELEPHONY_LOGI("Disable telephony observer");
    ASSERT_TRUE(telephonyObserver0_ != nullptr);
    Telephony::TelephonyObserverClient::GetInstance().RemoveStateObserver(
        DEFAULT_SIM_SLOT_ID, Telephony::TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE |
                                 Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE |
                                 Telephony::TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO |
                                 Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS |
                                 Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE |
                                 Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE |
                                 Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW);
    telephonyObserver0_ = nullptr;
    ASSERT_TRUE(telephonyObserver1_ != nullptr);
    Telephony::TelephonyObserverClient::GetInstance().RemoveStateObserver(
        SIM_SLOT_ID_1, Telephony::TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE |
                           Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE |
                           Telephony::TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO |
                           Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS |
                           Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE |
                           Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE |
                           Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW);
    telephonyObserver1_ = nullptr;
}

void StateRegistryTest::SetUp(void)
{
    // step 3: input testcase setup step
}

void StateRegistryTest::TearDown(void)
{
    // step 3: input testcase teardown step
}

bool StateRegistryTest::HasSimCard(int32_t slotId)
{
    bool hasSimCard = false;
    CoreServiceClient::GetInstance().HasSimCard(slotId, hasSimCard);
    return hasSimCard;
}

void StateRegistryTest::UpdateCallState(int32_t slotId)
{
    AccessToken token;
    int32_t callState = 16;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    int32_t ret =
        DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateCallState(slotId, callState, number);
    TELEPHONY_LOGI("StateRegistryTest::UpdateCallState ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateCallStateForSlotId(int32_t slotId)
{
    AccessToken token;
    int32_t callState = 16;
    int32_t callId = 0;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateCallStateForSlotId(
        slotId, callId, callState, number);
    TELEPHONY_LOGI("StateRegistryTest::UpdateCallStateForSlotId ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateSignalInfo(int32_t slotId)
{
    AccessToken token;
    std::vector<sptr<SignalInformation>> vec;
    std::unique_ptr<SignalInformation> signal = std::make_unique<GsmSignalInformation>();
    ASSERT_TRUE(signal != nullptr);
    vec.push_back(signal.release());
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateSignalInfo(slotId, vec);
    TELEPHONY_LOGI("StateRegistryTest::UpdateSignalInfo result = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateCellularDataConnectState(int32_t slotId)
{
    AccessToken token;
    int32_t dataState = 1;
    int32_t networkState = 1;
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateCellularDataConnectState(
        slotId, dataState, networkState);
    TELEPHONY_LOGI("StateRegistryTest::UpdateCellularDataConnectState ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateCellularDataFlow(int32_t slotId)
{
    AccessToken token;
    int32_t dataFlowType = 0;
    int32_t ret =
        DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateCellularDataFlow(slotId, dataFlowType);
    TELEPHONY_LOGI("StateRegistryTest::UpdateCellularDataFlow ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateSimState(int32_t slotId)
{
    AccessToken token;
    CardType type = CardType::UNKNOWN_CARD;
    SimState state = SimState::SIM_STATE_UNKNOWN;
    LockReason reason = LockReason::SIM_NONE;
    int32_t ret =
        DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateSimState(slotId, type, state, reason);
    TELEPHONY_LOGI("StateRegistryTest::UpdateSimState ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateNetworkState(int32_t slotId)
{
    AccessToken token;
    std::unique_ptr<NetworkState> networkState = std::make_unique<NetworkState>();
    ASSERT_TRUE(networkState != nullptr);
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateNetworkState(
        slotId, networkState.release());
    TELEPHONY_LOGI("StateRegistryTest::UpdateNetworkState ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateCfuIndicator(int32_t slotId)
{
    AccessToken token;
    bool cfuResult = true;
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateCfuIndicator(
        slotId, cfuResult);
    TELEPHONY_LOGI("StateRegistryTest::UpdateCfuIndicator ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateVoiceMailMsgIndicator(int32_t slotId)
{
    AccessToken token;
    bool voiceMailMsgResult = true;
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateVoiceMailMsgIndicator(
        slotId, voiceMailMsgResult);
    TELEPHONY_LOGI("StateRegistryTest::UpdateVoiceMailMsgIndicator ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

void StateRegistryTest::UpdateIccAccount()
{
    int32_t ret = DelayedRefSingleton<TelephonyStateRegistryClient>::GetInstance().UpdateIccAccount();
    TELEPHONY_LOGI("StateRegistryTest::UpdateIccAccount ret = %{public}d", ret);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

#ifndef TEL_TEST_UNSUPPORT
/**
 * @tc.number   StateRegistry_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, StateRegistry_001, Function | MediumTest | Level0)
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_TRUE(systemAbilityMgr != nullptr);
    auto remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    ASSERT_TRUE(remote != nullptr);
    auto stateRegistryService = iface_cast<ITelephonyStateNotify>(remote);
    ASSERT_TRUE(stateRegistryService != nullptr);
    TELEPHONY_LOGI("HWTEST_F StateRegistry_001");
}

/**
 * @tc.number   UpdateCallState_001
 * @tc.name     update call state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCallState_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateCallState(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateCallState_002
 * @tc.name     update call state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCallState_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateCallState(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateCallStateForSlotId_001
 * @tc.name     update call state by slotId
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCallStateForSlotId_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateCallStateForSlotId(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateCallStateForSlotId_002
 * @tc.name     update call state by slotId
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCallStateForSlotId_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateCallStateForSlotId(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateSignalInfo_001
 * @tc.name     update signal info
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateSignalInfo_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateSignalInfo(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateSignalInfo_002
 * @tc.name     update signal info
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateSignalInfo_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateSignalInfo(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateCellularDataConnectState_001
 * @tc.name     update cellular data connect state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCellularDataConnectState_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateCellularDataConnectState(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateCellularDataConnectState_002
 * @tc.name     update cellular data connect state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCellularDataConnectState_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateCellularDataConnectState(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateCellularDataFlow_001
 * @tc.name     update cellular flow data
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCellularDataFlow_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateCellularDataFlow(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateCellularDataFlow_002
 * @tc.name     update cellular flow data
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCellularDataFlow_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateCellularDataFlow(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateSimState_001
 * @tc.name     update sim state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateSimState_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateSimState(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateSimState_002
 * @tc.name     update sim state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateSimState_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateSimState(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateNetworkState_001
 * @tc.name     update network state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateNetworkState_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateNetworkState(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateNetworkState_002
 * @tc.name     update network state
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateNetworkState_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateNetworkState(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateCfuIndicator_001
 * @tc.name     update the result of call forwarding
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCfuIndicator_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateCfuIndicator(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateCfuIndicator_002
 * @tc.name     update the result of call forwarding
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateCfuIndicator_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateCfuIndicator(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateVoiceMailMsgIndicator_001
 * @tc.name     update voice mail message indicator
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateVoiceMailMsgIndicator_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateVoiceMailMsgIndicator(DEFAULT_SIM_SLOT_ID);
}

/**
 * @tc.number   UpdateVoiceMailMsgIndicator_002
 * @tc.name     update voice mail message indicator
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateVoiceMailMsgIndicator_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateVoiceMailMsgIndicator(SIM_SLOT_ID_1);
}

/**
 * @tc.number   UpdateIccAccount_001
 * @tc.name     Update Icc Account
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateIccAccount_001, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    UpdateIccAccount();
}

/**
 * @tc.number   UpdateIccAccount_002
 * @tc.name     UpdateIccAccount
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, UpdateIccAccount_002, Function | MediumTest | Level1)
{
    if (!StateRegistryTest::HasSimCard(SIM_SLOT_ID_1)) {
        return;
    }
    UpdateIccAccount();
}

/**
 * @tc.number   TelephonyStateManagerTest_001
 * @tc.name     telephony state manager add/remove state observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateManagerTest_001, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyStateManagerTest_001 start!");
    if (!StateRegistryTest::HasSimCard(DEFAULT_SIM_SLOT_ID)) {
        return;
    }
    TelephonyStateManager::AddStateObserver(
        telephonyObserver0_, DEFAULT_SIM_SLOT_ID, TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE, true);
    wptr<IRemoteObject> wptrDeath = nullptr;
    int32_t ret = TelephonyStateManager::RemoveStateObserver(
        DEFAULT_SIM_SLOT_ID, Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_001
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_001, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_001 start!");
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    std::u16string testStr = u"test";
    if (!dataParcel.WriteInterfaceToken(testStr)) {
        TELEPHONY_LOGE("TelephonyObserverTest_001 WriteInterfaceToken failed!");
        return;
    }
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_CALL_STATE_UPDATED), dataParcel, reply,
        option);
    EXPECT_EQ(TELEPHONY_ERR_DESCRIPTOR_MISMATCH, ret);
}

/**
 * @tc.number   TelephonyObserverTest_002
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_002, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_002 start!");
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    std::u16string phoneNumber = u"123456";
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_002 WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    dataParcel.WriteInt32(CALL_STATUS_ACTIVE);
    dataParcel.WriteString16(phoneNumber);
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_CALL_STATE_UPDATED), dataParcel, reply,
        option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_003
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_003, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_003 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_003 WriteInterfaceToken failed!");
        return;
    }
    std::vector<sptr<SignalInformation>> vec;
    std::unique_ptr<SignalInformation> gsmSignal = std::make_unique<GsmSignalInformation>();
    ASSERT_TRUE(gsmSignal != nullptr);
    vec.push_back(gsmSignal.release());
    std::unique_ptr<WcdmaSignalInformation> wcdmaSignal = std::make_unique<WcdmaSignalInformation>();
    ASSERT_TRUE(wcdmaSignal != nullptr);
    vec.push_back(wcdmaSignal.release());
    std::unique_ptr<TdScdmaSignalInformation> tdScdmaSignal = std::make_unique<TdScdmaSignalInformation>();
    ASSERT_TRUE(tdScdmaSignal != nullptr);
    vec.push_back(tdScdmaSignal.release());
    std::unique_ptr<CdmaSignalInformation> cdmaSignal = std::make_unique<CdmaSignalInformation>();
    ASSERT_TRUE(cdmaSignal != nullptr);
    vec.push_back(cdmaSignal.release());
    std::unique_ptr<LteSignalInformation> lteSignal = std::make_unique<LteSignalInformation>();
    ASSERT_TRUE(lteSignal != nullptr);
    vec.push_back(lteSignal.release());
    std::unique_ptr<NrSignalInformation> nrSignal = std::make_unique<NrSignalInformation>();
    ASSERT_TRUE(nrSignal != nullptr);
    vec.push_back(nrSignal.release());
    int32_t size = static_cast<int32_t>(vec.size());
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    dataParcel.WriteInt32(size);
    for (const auto &v : vec) {
        v->Marshalling(dataParcel);
    }
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_SIGNAL_INFO_UPDATED), dataParcel, reply,
        option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_004
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_004, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_004 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_004 WriteInterfaceToken failed!");
        return;
    }
    std::vector<sptr<CellInformation>> vec;
    std::unique_ptr<GsmCellInformation> gsmCell = std::make_unique<GsmCellInformation>();
    ASSERT_TRUE(gsmCell != nullptr);
    vec.push_back(gsmCell.release());
    std::unique_ptr<LteCellInformation> lteCell = std::make_unique<LteCellInformation>();
    ASSERT_TRUE(lteCell != nullptr);
    vec.push_back(lteCell.release());
    std::unique_ptr<WcdmaCellInformation> wcdmaCell = std::make_unique<WcdmaCellInformation>();
    ASSERT_TRUE(wcdmaCell != nullptr);
    vec.push_back(wcdmaCell.release());
    std::unique_ptr<NrCellInformation> nrCell = std::make_unique<NrCellInformation>();
    ASSERT_TRUE(nrCell != nullptr);
    vec.push_back(nrCell.release());
    int32_t size = static_cast<int32_t>(vec.size());
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    if (!dataParcel.WriteInt32(size)) {
        TELEPHONY_LOGE("Failed to write Cellinformation array size!");
        return;
    }
    for (const auto &v : vec) {
        v->Marshalling(dataParcel);
    }
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_CELL_INFO_UPDATED), dataParcel, reply,
        option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_005
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_005, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_005 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_005 WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    std::unique_ptr<NetworkState> networkState = std::make_unique<NetworkState>();
    ASSERT_TRUE(networkState != nullptr);
    (networkState.release())->Marshalling(dataParcel);
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_NETWORK_STATE_UPDATED), dataParcel, reply,
        option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_006
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_006, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_006 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_006 WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    dataParcel.WriteInt32(SINGLE_MODE_SIM_CARD);
    dataParcel.WriteInt32(SIM_STATE_NOT_PRESENT);
    dataParcel.WriteInt32(SIM_PIN);
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_SIM_STATE_UPDATED), dataParcel, reply,
        option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_007
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_007, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_007 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_007 WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    dataParcel.WriteInt32(DATA_STATE_CONNECTING);
    dataParcel.WriteInt32(NETWORK_TYPE_GSM);
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_CELLULAR_DATA_CONNECT_STATE_UPDATED),
        dataParcel, reply, option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_008
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_008, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_008 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_008 WriteInterfaceToken failed!");
        return;
    }
    dataParcel.WriteInt32(DEFAULT_SIM_SLOT_ID);
    dataParcel.WriteInt32(DATA_FLOW_TYPE_DOWN);
    int32_t ret = telephonyObserver.OnRemoteRequest(
        static_cast<uint32_t>(TelephonyObserverBroker::ObserverBrokerCode::ON_CELLULAR_DATA_FLOW_UPDATED), dataParcel,
        reply, option);
    EXPECT_EQ(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_009
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_009, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyObserverTest_009 start!");
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel reply;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (!dataParcel.WriteInterfaceToken(TelephonyObserverProxy::GetDescriptor())) {
        TELEPHONY_LOGE("TelephonyObserverTest_009 WriteInterfaceToken failed!");
        return;
    }
    uint32_t testId = 123;
    int32_t ret = telephonyObserver.OnRemoteRequest(testId, dataParcel, reply, option);
    EXPECT_NE(TELEPHONY_ERR_SUCCESS, ret);
}

/**
 * @tc.number   TelephonyObserverTest_010
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_010, Function | MediumTest | Level1)
{
    int32_t signalSize = 5;
    int32_t cellInfoMaxSize = 11;
    int32_t callState = 16;
    std::u16string number = u"137xxxxxxxx";
    std::unique_ptr<Telephony::TelephonyObserverClient> telephonyObserverClient =
        std::make_unique<Telephony::TelephonyObserverClient>();
    telephonyObserverClient->OnRemoteDied(nullptr);
    telephonyObserverClient->proxy_ = nullptr;
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    telephonyObserverClient->OnRemoteDied(obj);
    telephonyObserverClient->proxy_ = iface_cast<ITelephonyStateNotify>(obj);
    telephonyObserverClient->OnRemoteDied(obj);
    std::shared_ptr<OHOS::Telephony::TelephonyObserverProxy> telephonyObserverProxy =
        std::make_shared<OHOS::Telephony::TelephonyObserverProxy>(obj);
    telephonyObserverProxy->OnCallStateUpdated(DEFAULT_SIM_SLOT_ID, callState, number);
    std::vector<sptr<SignalInformation>> signalInformations;
    telephonyObserverProxy->OnSignalInfoUpdated(DEFAULT_SIM_SLOT_ID, signalInformations);
    for (int32_t i = 0; i < signalSize; i++) {
        sptr<SignalInformation> signalInformation = sptr<SignalInformation>();
        signalInformations.push_back(signalInformation);
    }
    telephonyObserverProxy->OnSignalInfoUpdated(DEFAULT_SIM_SLOT_ID, signalInformations);
    std::vector<sptr<CellInformation>> cellInformations;
    telephonyObserverProxy->OnCellInfoUpdated(DEFAULT_SIM_SLOT_ID, cellInformations);
    for (int32_t i = 0; i < cellInfoMaxSize; i++) {
        sptr<CellInformation> cellInfo = sptr<CellInformation>();
        cellInformations.push_back(cellInfo);
    }
    telephonyObserverProxy->OnCellInfoUpdated(DEFAULT_SIM_SLOT_ID, cellInformations);
    EXPECT_TRUE(telephonyObserverClient != nullptr);
    EXPECT_TRUE(telephonyObserverProxy != nullptr);
    EXPECT_GE(signalInformations.size(), static_cast<size_t>(0));
    EXPECT_GE(cellInformations.size(), static_cast<size_t>(0));
}

/**
 * @tc.number   TelephonyObserverTest_011
 * @tc.name     telephony observer test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyObserverTest_011, Function | MediumTest | Level1)
{
    int32_t slotId = 0;
    bool cfuResult = false;
    std::shared_ptr<OHOS::Telephony::TelephonyObserver> telephonyObserver =
        std::make_shared<OHOS::Telephony::TelephonyObserver>();
    telephonyObserver->OnCfuIndicatorUpdated(slotId, cfuResult);
    telephonyObserver->OnVoiceMailMsgIndicatorUpdated(slotId, cfuResult);
    MessageParcel data;
    MessageParcel reply;
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    telephonyObserver->OnIccAccountUpdatedInner(data, reply);
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    std::shared_ptr<OHOS::Telephony::TelephonyObserverProxy> telephonyObserverProxy =
        std::make_shared<OHOS::Telephony::TelephonyObserverProxy>(obj);
    telephonyObserverProxy->OnIccAccountUpdated();
    EXPECT_TRUE(telephonyObserver != nullptr);
    EXPECT_TRUE(telephonyObserverProxy != nullptr);
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_001
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_001, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_001 service is nullptr");
        return;
    }
    EXPECT_TRUE(service->IsCommonEventServiceAbilityExist());
    EXPECT_EQ(TELEPHONY_ERROR, service->GetLockReason(0));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCellularDataConnectionNetworkType(0));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCellularDataFlow(0));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCellularDataConnectionState(0));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCardType(0));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCallState(0));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetSimState(0));
    service->simReason_[0] = LockReason::SIM_NONE;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetLockReason(1));
    EXPECT_NE(TELEPHONY_ERROR, service->GetLockReason(0));
    service->cellularDataConnectionNetworkType_[0] = 0;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCellularDataConnectionNetworkType(1));
    EXPECT_EQ(0, service->GetCellularDataConnectionNetworkType(0));
    service->cellularDataFlow_[0] = 0;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCellularDataFlow(1));
    EXPECT_EQ(0, service->GetCellularDataFlow(0));
    service->cellularDataConnectionState_[0] = 0;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCellularDataConnectionState(1));
    EXPECT_EQ(0, service->GetCellularDataConnectionState(0));
    service->cardType_[0] = CardType::UNKNOWN_CARD;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCardType(1));
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCardType(0));
    service->callState_[0] = 0;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetCallState(1));
    EXPECT_EQ(0, service->GetCallState(0));
    service->simState_[0] = SimState::SIM_STATE_UNKNOWN;
    EXPECT_EQ(TELEPHONY_ERROR, service->GetSimState(1));
    EXPECT_EQ(0, service->GetSimState(0));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_002
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_002, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_002 service is nullptr");
        return;
    }
    int32_t fd = -1;
    std::vector<std::u16string> args;
    EXPECT_EQ(TELEPHONY_ERR_FAIL, service->Dump(fd, args));
    fd = 1;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->Dump(fd, args));
    sptr<NetworkState> networkState = nullptr;
    service->SendNetworkStateChanged(0, networkState);
    TELEPHONY_EXT_WRAPPER.sendNetworkStateChanged_ = nullptr;
    networkState = std::make_unique<NetworkState>().release();
    service->SendNetworkStateChanged(0, networkState);
    std::vector<sptr<SignalInformation>> vec;
    service->SendSignalInfoChanged(0, vec);
    TELEPHONY_EXT_WRAPPER.sendNetworkStateChanged_ = nullptr;
    vec.push_back(std::make_unique<LteSignalInformation>().release());
    service->SendSignalInfoChanged(0, vec);
    std::u16string number = u"123456";
    service->SendCallStateChanged(0, 0, number);
    TelephonyStateRegistryRecord record;
    service->UpdateData(record);
    record.telephonyObserver_ = std::make_unique<TelephonyObserver>().release();
    service->UpdateData(record);
    record.mask_ = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE |
                   TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE |
                   TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO |
                   TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS |
                   TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE |
                   TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE |
                   TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW |
                   TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR |
                   TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR |
                   TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT;
    service->UpdateData(record);
    EXPECT_EQ(Str8ToStr16(""), service->GetCallIncomingNumberForSlotId(record, 0));
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapper();
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_003
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_003, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_003 service is nullptr");
        return;
    }
    EXPECT_TRUE(service->CheckPermission(0));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(0));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR));
    uint32_t mask = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
    int32_t tokenId = 123456789;
    pid_t pid = 1234;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UnregisterStateChange(0, mask, tokenId, pid));
    mask = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UnregisterStateChange(0, mask, tokenId, pid));

    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    service->stateRecords_[0].tokenId_ = 123456789;
    service->stateRecords_[0].pid_ = 1234;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UnregisterStateChange(0, 0, tokenId, pid));
    service->stateRecords_[0].tokenId_ = 123456788;
    EXPECT_EQ(TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST, service->UnregisterStateChange(0, 0, tokenId, pid));
    service->stateRecords_[0].tokenId_ = -1;
    EXPECT_EQ(TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST, service->UnregisterStateChange(0, 0, tokenId, pid));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW;
    EXPECT_EQ(TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST, service->UnregisterStateChange(0, 0, tokenId, pid));
    service->stateRecords_[0].slotId_ = 1;
    EXPECT_EQ(TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST, service->UnregisterStateChange(0, 0, tokenId, pid));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_004
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_004, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_004 service is nullptr");
        return;
    }
    if (service->state_ == ServiceRunningState::STATE_RUNNING) {
        service->OnStart();
    }
    sptr<TelephonyObserverBroker> telephonyObserver = nullptr;
    uint32_t mask = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED,
        service->RegisterStateChange(telephonyObserver, 0, mask, "", true, 0, 0, 0));
    mask = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED,
        service->RegisterStateChange(telephonyObserver, 0, mask, "", true, 0, 0, 0));
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    int32_t invalidSlotId = 5;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, invalidSlotId, 0, "", true, 0, 0, 0));
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, 0, 0, "", true, 0, 0, 0));
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, 0, 0, "", false, 0, 0, 0));
    service->stateRecords_[0].tokenId_ = 1;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, 0, 0, "", true, 0, 0, 0));
    service->stateRecords_[0].tokenId_ = 123456789;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, 0, 0, "", true, 0, 0, 0));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, 0, 0, "", true, 0, 0, 0));
    service->stateRecords_[0].slotId_ = 1;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->RegisterStateChange(telephonyObserver, 0, 0, "", true, 0, 0, 0));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_005
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_005, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_005 service is nullptr");
        return;
    }
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    int32_t invalidSlotId = 5;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateVoiceMailMsgIndicator(invalidSlotId, true));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateVoiceMailMsgIndicator(0, true));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateIccAccount());
    service->stateRecords_[0].telephonyObserver_ = std::make_unique<TelephonyObserver>().release();
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateIccAccount());
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateIccAccount());
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateCfuIndicator(invalidSlotId, true));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateCfuIndicator(0, true));
    sptr<NetworkState> networkState = std::make_unique<NetworkState>().release();
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateNetworkState(invalidSlotId, networkState));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateNetworkState(0, networkState));
    std::vector<sptr<CellInformation>> vecCellInfo;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateCellInfo(invalidSlotId, vecCellInfo));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateCellInfo(0, vecCellInfo));
    std::vector<sptr<SignalInformation>> vecSignalInfo;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateSignalInfo(invalidSlotId, vecSignalInfo));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateSignalInfo(0, vecSignalInfo));
    CardType type = CardType::UNKNOWN_CARD;
    SimState state = SimState::SIM_STATE_UNKNOWN;
    LockReason reason = LockReason::SIM_NONE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateSimState(invalidSlotId, type, state, reason));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateSimState(0, type, state, reason));
    std::u16string number = u"123";
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateCallStateForSlotId(invalidSlotId, 0, 0, number));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateCallStateForSlotId(0, 0, 0, number));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateCallState(invalidSlotId, 0, number));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateCallState(0, 0, number));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateCellularDataFlow(invalidSlotId, 0));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateCellularDataFlow(0, 0));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_SLODID_ERROR, service->UpdateCellularDataConnectState(invalidSlotId, 0, 0));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED, service->UpdateCellularDataConnectState(0, 0, 0));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_006
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_006, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_006 service is nullptr");
        return;
    }
    AccessToken token;
    std::u16string number = u"123";
    TelephonyStateRegistryRecord record;
    EXPECT_TRUE(record.IsCanReadCallHistory());
    service->stateRecords_.push_back(record);
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCellularDataConnectState(0, 0, 0));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCellularDataFlow(0, 0));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCallStateForSlotId(0, 0, 0, number));
    CardType type = CardType::UNKNOWN_CARD;
    SimState state = SimState::SIM_STATE_UNKNOWN;
    LockReason reason = LockReason::SIM_NONE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateSimState(0, type, state, reason));

    service->stateRecords_[0].telephonyObserver_ = std::make_unique<TelephonyObserver>().release();
    service->stateRecords_[0].slotId_ = 3;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCellularDataConnectState(0, 0, 0));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCellularDataFlow(0, 0));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCallState(0, 0, number));
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCallStateForSlotId(0, 0, 0, number));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateSimState(0, type, state, reason));

    service->stateRecords_[0].slotId_ = 0;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCellularDataConnectState(0, 0, 0));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCellularDataFlow(0, 0));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCallState(0, 0, number));
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCallStateForSlotId(0, 0, 0, number));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateSimState(0, type, state, reason));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_007
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_007, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_007 service is nullptr");
        return;
    }
    AccessToken token;
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    sptr<NetworkState> networkState = std::make_unique<NetworkState>().release();
    std::vector<sptr<SignalInformation>> vecSignalInfo;
    EXPECT_NE(TELEPHONY_SUCCESS, service->UpdateSignalInfo(0, vecSignalInfo));
    std::vector<sptr<CellInformation>> vecCellInfo;
    EXPECT_NE(TELEPHONY_SUCCESS, service->UpdateCellInfo(0, vecCellInfo));
    EXPECT_NE(TELEPHONY_SUCCESS, service->UpdateNetworkState(0, networkState));
    EXPECT_NE(TELEPHONY_SUCCESS, service->UpdateCfuIndicator(0, true));
    EXPECT_NE(TELEPHONY_SUCCESS, service->UpdateVoiceMailMsgIndicator(0, true));

    service->stateRecords_[0].telephonyObserver_ = std::make_unique<TelephonyObserver>().release();
    service->stateRecords_[0].slotId_ = 3;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateSignalInfo(0, vecSignalInfo));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCellInfo(0, vecCellInfo));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateNetworkState(0, networkState));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateCfuIndicator(0, true));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR;
    EXPECT_EQ(TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST, service->UpdateVoiceMailMsgIndicator(0, true));

    service->stateRecords_[0].slotId_ = 0;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateSignalInfo(0, vecSignalInfo));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCellInfo(0, vecCellInfo));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateNetworkState(0, networkState));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCfuIndicator(0, true));
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateVoiceMailMsgIndicator(0, true));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_008
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_008, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_008 service is nullptr");
        return;
    }
    AccessToken token;
    MessageParcel dataParcel;
    dataParcel.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::GSM));
    dataParcel.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::CDMA));
    dataParcel.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::LTE));
    dataParcel.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::NR));
    dataParcel.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::WCDMA));
    dataParcel.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::UNKNOWN));
    int32_t size = 6;
    std::vector<sptr<SignalInformation>> result;
    service->parseSignalInfos(dataParcel, size, result);
    MessageParcel dataSignal;
    service->ParseLteNrSignalInfos(dataSignal, result, SignalInformation::NetworkType::UNKNOWN);
    MessageParcel dataSignalNr;
    dataSignalNr.WriteInt32(0);
    dataSignalNr.WriteInt32(0);
    dataSignalNr.WriteInt32(0);
    service->ParseLteNrSignalInfos(dataSignalNr, result, SignalInformation::NetworkType::NR);
    MessageParcel dataSignalLte;
    dataSignalLte.WriteInt32(0);
    dataSignalLte.WriteInt32(0);
    dataSignalLte.WriteInt32(0);
    dataSignalLte.WriteInt32(0);
    service->ParseLteNrSignalInfos(dataSignalLte, result, SignalInformation::NetworkType::LTE);

    TelephonyStateRegistryRecord record;
    std::u16string testNumber = u"123";
    service->callIncomingNumber_[0] = testNumber;
    EXPECT_EQ(testNumber, service->GetCallIncomingNumberForSlotId(record, 0));
    EXPECT_TRUE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE));
    EXPECT_TRUE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR));
    EXPECT_TRUE(service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_009
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_009, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_009 service is nullptr");
        return;
    }
    MessageParcel dataCellInfo;
    int32_t size = 4;
    dataCellInfo.WriteInt32(0);
    dataCellInfo.WriteInt32(size);
    dataCellInfo.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::GSM));
    dataCellInfo.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::LTE));
    dataCellInfo.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::NR));
    dataCellInfo.WriteInt32(static_cast<int32_t>(SignalInformation::NetworkType::UNKNOWN));
    std::vector<sptr<SignalInformation>> result;
    MessageParcel reply;
    service->OnUpdateCellInfo(dataCellInfo, reply);

    MessageParcel dataNetworkState;
    dataNetworkState.WriteInt32(0);
    service->OnUpdateNetworkState(dataNetworkState, reply);
    MessageParcel dataRegisterState;
    dataNetworkState.WriteInt32(0);
    dataNetworkState.WriteInt32(0);
    dataNetworkState.WriteBool(true);
    service->OnRegisterStateChange(dataRegisterState, reply);

    MessageParcel dataRead;
    sptr<TelephonyObserverBroker> callback = nullptr;
    EXPECT_NE(TELEPHONY_SUCCESS, service->ReadData(dataRead, reply, callback));
}

/**
 * @tc.number   TelephonyStateRegistryServiceTest_010
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistryServiceTest_010, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_010 service is nullptr");
        return;
    }
    AccessToken token;
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    service->stateRecords_[0].telephonyObserver_ = std::make_unique<TelephonyObserver>().release();
    service->stateRecords_[0].slotId_ = 0;
    TELEPHONY_EXT_WRAPPER.onSignalInfoUpdated_ = nullptr;
    std::vector<sptr<SignalInformation>> vecSignalInfo;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateSignalInfo(0, vecSignalInfo));
    TELEPHONY_EXT_WRAPPER.onCellInfoUpdated_ = nullptr;
    std::vector<sptr<CellInformation>> vecCellInfo;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateCellInfo(0, vecCellInfo));
    TELEPHONY_EXT_WRAPPER.onNetworkStateUpdated_ = nullptr;
    sptr<NetworkState> networkState = std::make_unique<NetworkState>().release();
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
    EXPECT_EQ(TELEPHONY_SUCCESS, service->UpdateNetworkState(0, networkState));
    TELEPHONY_EXT_WRAPPER.InitTelephonyExtWrapper();
}

/**
 * @tc.number   TelephonyStateRegistrySimCountTest_001
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistrySimCountTest_001, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_001 service is nullptr");
        return;
    }
    service->slotSize_ = MAX_SLOT_COUNT;
    EXPECT_TRUE(service->VerifySlotId(0));
    EXPECT_TRUE(service->VerifySlotId(1));
    EXPECT_TRUE(service->VerifySlotId(2));
    EXPECT_FALSE(service->VerifySlotId(3));
    EXPECT_FALSE(service->VerifySlotId(-1));
}

/**
 * @tc.number   TelephonyStateRegistrySimCountTest_002
 * @tc.name     telephony state registry service test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateRegistrySimCountTest_002, Function | MediumTest | Level1)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    if (service == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryServiceTest_001 service is nullptr");
        return;
    }
    service->slotSize_ = DUAL_SLOT_COUNT;
    EXPECT_TRUE(service->VerifySlotId(0));
    EXPECT_TRUE(service->VerifySlotId(1));
    EXPECT_FALSE(service->VerifySlotId(2));
    EXPECT_FALSE(service->VerifySlotId(3));
    EXPECT_FALSE(service->VerifySlotId(-1));
}

#else // TEL_TEST_UNSUPPORT
/**
 * @tc.number   State_MockTest_001
 * @tc.name     Mock test for unsupport platform
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, State_MockTest_001, Function | MediumTest | Level1)
{
    EXPECT_TRUE(true);
}
#endif // TEL_TEST_UNSUPPORT
} // namespace Telephony
} // namespace OHOS
