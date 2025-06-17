/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "state_registry_branch_test.h"

#include "core_service_client.h"
#include "sim_state_type.h"
#include "state_registry_test.h"
#include "telephony_ext_wrapper.h"
#include "telephony_log_wrapper.h"
#include "telephony_observer_client.h"
#include "telephony_observer_proxy.h"
#include "telephony_state_manager.h"
#include "telephony_state_registry_client.h"
#include "telephony_state_registry_proxy.h"
#include "telephony_state_registry_service.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
static constexpr int32_t DATA_STATE_CONNECTING = 1;
static constexpr int32_t NETWORK_TYPE_GSM = 1;
static constexpr int32_t DATA_FLOW_TYPE_DOWN = 1;
static constexpr int32_t PROFILE_STATE_DISCONNECTING = 3;
class StateRegistryBranchTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void StateRegistryBranchTest::SetUpTestCase(void)
{
    ASSERT_TRUE(CoreServiceClient::GetInstance().GetProxy() != nullptr);
}

void StateRegistryBranchTest::TearDownTestCase(void)
{
}

void StateRegistryBranchTest::SetUp(void) {}

void StateRegistryBranchTest::TearDown(void) {}

class TestIRemoteObject : public IRemoteObject {
public:
    uint32_t requestCode_ = -1;
    int32_t result_ = 0;

public:
    TestIRemoteObject() : IRemoteObject(u"test_remote_object") {}

    ~TestIRemoteObject() {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        TELEPHONY_LOGI("Mock SendRequest");
        requestCode_ = code;
        reply.WriteInt32(result_);
        return 0;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

/**
 * @tc.number   StateRegistry_Notify_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, StateRegistry_Notify_001, Function | MediumTest | Level0)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(sam, nullptr);
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    ASSERT_NE(obj, nullptr);
    std::shared_ptr<OHOS::Telephony::TelephonyObserverProxy> telephonyObserverProxy =
        std::make_shared<OHOS::Telephony::TelephonyObserverProxy>(obj);
    CardType type = CardType::UNKNOWN_CARD;
    SimState state = SimState::SIM_STATE_UNKNOWN;
    LockReason reason = LockReason::SIM_NONE;
    telephonyObserverProxy->OnSimStateUpdated(0, type, state, reason);
    std::vector<sptr<CellInformation>> vec;
    std::unique_ptr<GsmCellInformation> gsmCell = std::make_unique<GsmCellInformation>();
    vec.push_back(gsmCell.release());
    telephonyObserverProxy->OnCellInfoUpdated(0, vec);
    std::unique_ptr<NetworkState> networkState = std::make_unique<NetworkState>();
    telephonyObserverProxy->OnNetworkStateUpdated(0, networkState.release());
    telephonyObserverProxy->OnCellularDataConnectStateUpdated(0, DATA_STATE_CONNECTING, NETWORK_TYPE_GSM);
    telephonyObserverProxy->OnCellularDataFlowUpdated(0, DATA_FLOW_TYPE_DOWN);
    telephonyObserverProxy->OnCfuIndicatorUpdated(0, true);
    telephonyObserverProxy->OnVoiceMailMsgIndicatorUpdated(0, true);
    ASSERT_NE(telephonyObserverProxy, nullptr);
}

/**
 * @tc.number   TelephonyStateRegistryService_OnStart_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, TelephonyStateRegistryService_OnStart_001, TestSize.Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    service->state_ = ServiceRunningState::STATE_RUNNING;
    service->OnStart();
    ASSERT_EQ(static_cast<int32_t>(service->state_), static_cast<int32_t>(ServiceRunningState::STATE_RUNNING));
}

/**
 * @tc.number   TelephonyStateRegistryService_OnStart_002
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, TelephonyStateRegistryService_OnStart_002, TestSize.Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    service->OnStart();
    service->OnStop();
    ASSERT_EQ(static_cast<int32_t>(service->state_), static_cast<int32_t>(ServiceRunningState::STATE_STOPPED));
}

/**
 * @tc.number   TelephonyStateRegistryService_UpdateIccAccount_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, TelephonyStateRegistryService_UpdateIccAccount_001, TestSize.Level0)
{
    AccessToken token;
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT;
    service->stateRecords_[0].telephonyObserver_ = nullptr;
    auto result = service->UpdateIccAccount();
    ASSERT_EQ(result, TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
}

/**
 * @tc.number   TelephonyStateRegistryService_UpdateIccAccount_002
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, TelephonyStateRegistryService_UpdateIccAccount_002, TestSize.Level0)
{
    AccessToken token;
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT;
    service->stateRecords_[0].telephonyObserver_ = std::make_unique<TelephonyObserver>().release();
    auto result = service->UpdateIccAccount();
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
}

/**
 * @tc.number   TelephonyStateRegistryService_UpdateIccAccount_003
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, TelephonyStateRegistryService_UpdateIccAccount_003, TestSize.Level0)
{
    AccessToken token;
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
    auto result = service->UpdateIccAccount();
    ASSERT_EQ(result, TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
}

/**
 * @tc.number   TelephonyStateRegistryService_CheckCallerIsSystemApp_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, Service_CheckCallerIsSystemApp_001, TestSize.Level0)
{
    SecurityToken token;
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    bool result = service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO);
    ASSERT_EQ(result, false);
    result = service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR);
    ASSERT_EQ(result, false);
    result = service->CheckCallerIsSystemApp(TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR);
    ASSERT_EQ(result, false);
    pid_t pid = -1;
    auto ret = service->UnregisterStateChange(0, TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO, 0, pid);
    ASSERT_EQ(ret, TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API);
}

/**
 * @tc.number   TelephonyStateRegistryService_UnregisterStateChange_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, Service_UnregisterStateChange_001, TestSize.Level0)
{
    AccessToken token;
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    TelephonyStateRegistryRecord record;
    service->stateRecords_.push_back(record);
    service->stateRecords_[0].slotId_ = 1;
    pid_t pid = -1;
    auto result = service->UnregisterStateChange(0, TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO, 0, pid);
    ASSERT_EQ(result, TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST);
    service->stateRecords_[0].slotId_ = 0;
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW;
    result = service->UnregisterStateChange(0, TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO, 0, pid);
    ASSERT_EQ(result, TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST);
    service->stateRecords_[0].mask_ = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
    service->stateRecords_[0].tokenId_ = 1234;
    result = service->UnregisterStateChange(0, TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO, 0, pid);
    ASSERT_EQ(result, TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST);
    service->stateRecords_[0].pid_ = 1;
    result = service->UnregisterStateChange(0, TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO, 1234, pid);
    ASSERT_EQ(result, TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST);
}

/**
 * @tc.number   TelephonyStateRegistryService_UnregisterStateChange_001
 * @tc.name     Get System Services
 * @tc.desc     Function test
 */
HWTEST_F(StateRegistryBranchTest, UpdateCellularDataConnectState_001, TestSize.Level0)
{
    sptr<TestIRemoteObject> remote = new (std::nothrow) TestIRemoteObject();
    std::shared_ptr<TelephonyStateRegistryProxy> proxy = std::make_shared<TelephonyStateRegistryProxy>(remote);
    auto result = proxy->UpdateCellularDataConnectState(0, PROFILE_STATE_DISCONNECTING, 0);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    result = proxy->UpdateCellularDataFlow(0, 0);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    int32_t callState = 16;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    result = proxy->UpdateCallState(callState, number);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    result = proxy->UpdateCallStateForSlotId(0, callState, number);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    std::vector<sptr<SignalInformation>> vec;
    std::unique_ptr<SignalInformation> signal = std::make_unique<GsmSignalInformation>();
    ASSERT_TRUE(signal != nullptr);
    vec.push_back(signal.release());
    result = proxy->UpdateSignalInfo(0, vec);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    std::vector<sptr<CellInformation>> cellInfoList;
    sptr<CellInformation> gsmCellInformation = new GsmCellInformation();
    cellInfoList.push_back(gsmCellInformation);
    result = proxy->UpdateCellInfo(0, cellInfoList);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    sptr<NetworkState> networkState = std::make_unique<NetworkState>().release();
    result = proxy->UpdateNetworkState(0, networkState);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    CardType type = CardType::UNKNOWN_CARD;
    SimState state = SimState::SIM_STATE_UNKNOWN;
    LockReason reason = LockReason::SIM_NONE;
    result = proxy->UpdateSimState(0, type, state, reason);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    result = proxy->UnregisterStateChange(0, TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    result = proxy->UpdateCfuIndicator(0, true);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    result = proxy->UpdateVoiceMailMsgIndicator(0, true);
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
    result = proxy->UpdateIccAccount();
    ASSERT_EQ(result, TELEPHONY_SUCCESS);
}

} // namespace Telephony
} // namespace OHOS