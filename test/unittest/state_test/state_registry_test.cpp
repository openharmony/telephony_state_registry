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

#define private public
#define protected public
#include "state_registry_test.h"

#include "core_service_client.h"
#include "sim_state_type.h"
#include "telephony_log_wrapper.h"
#include "telephony_observer_client.h"
#include "telephony_observer_proxy.h"
#include "telephony_state_manager.h"
#include "telephony_state_registry_client.h"

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
                Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW,
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
            Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW,
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
    TELEPHONY_LOGI("StateRegistryTest::UpdateSignalInfo ret = %{public}d", ret);
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
 * @tc.number   TelephonyStateManagerTest_001
 * @tc.name     telephony state ,anager test
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryTest, TelephonyStateManagerTest_001, Function | MediumTest | Level1)
{
    TELEPHONY_LOGI("TelephonyStateManagerTest_001 start!");
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
