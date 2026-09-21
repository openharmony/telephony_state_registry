/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
 
#include <gtest/gtest.h>
 
#include "accesstoken_kit.h"
#include "gmock/gmock.h"
#include "mock_telephony_permission.h"
#include "state_registry_errors.h"
#include "telephony_observer.h"
#include "telephony_permission.h"
#include "telephony_observer_broker.h"
#include "telephony_state_registry_service.h"
#include "token_setproc.h"
#include "voip_call_state_info.h"
 
namespace OHOS {
namespace Telephony {
using namespace testing;
using namespace testing::ext;
using namespace Security::AccessToken;
 
namespace {
constexpr int32_t UNREGISTERED_TOKEN_ID = 1234;
} // namespace
 
class VoipStateSpyObserver : public TelephonyObserver {
public:
    VoipStateSpyObserver() = default;
    ~VoipStateSpyObserver() = default;
 
    void OnVoIPStateUpdated(const VoIPCallStateInfo &info) override
    {
        voipCallbackCount_++;
        lastVoipInfo_ = info;
    }
 
    int32_t voipCallbackCount_ = 0;
    VoIPCallStateInfo lastVoipInfo_;
};
 
class VoipStateDeviceToken {
public:
    VoipStateDeviceToken()
    {
        currentID_ = GetSelfTokenID();
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(voipInfoParams_, voipPolicyParams_);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(tokenIdEx.tokenIDEx);
    }
 
    ~VoipStateDeviceToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }
 
    AccessTokenID GetTokenId() const
    {
        return accessID_;
    }
 
private:
    HapInfoParams voipInfoParams_ = {
        .bundleName = "tel_state_registry_service_test",
        .userID = 1,
        .instIndex = 0,
        .appIDDesc = "test",
        .isSystemApp = false,
    };
    PermissionDef voipManageCallDef_ = {
        .permissionName = "ohos.permission.MANAGE_CALL_FOR_DEVICES",
        .bundleName = "tel_state_registry_service_test",
        .grantMode = 1, // SYSTEM_GRANT
        .label = "label",
        .labelId = 1,
        .description = "Test voip call state",
        .descriptionId = 1,
        .availableLevel = APL_SYSTEM_BASIC,
    };
    PermissionStateFull voipManageCallState_ = {
        .grantFlags = { 2 }, // PERMISSION_USER_SET
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .isGeneral = true,
        .permissionName = "ohos.permission.MANAGE_CALL_FOR_DEVICES",
        .resDeviceID = { "local" },
    };
    HapPolicyParams voipPolicyParams_ = {
        .apl = APL_SYSTEM_BASIC,
        .domain = "test.domain",
        .permList = { voipManageCallDef_ },
        .permStateList = { voipManageCallState_ },
    };
 
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};
 
class TelephonyStateRegistryServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
 
    void SetUp(void)
    {
        permission_ = MockTelephonyPermission::GetOrCreateMockTelephonyPermission();
    }
 
    void TearDown(void)
    {
        auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
        if (service != nullptr) {
            service->stateRecords_.clear();
        }
        MockTelephonyPermission::ReleaseMockTelephonyPermission();
        permission_ = nullptr;
    }
 
protected:
    VoIPCallStateInfo BuildVoipInfo()
    {
        VoIPCallStateInfo info;
        info.appName = "com.example.voip";
        info.contactName = "Bob";
        info.callType = VoIPCallType::VIDEO_CONFERENCE;
        info.callState = VoIPCallState::ANSWERED;
        info.isVoiceAnswerSupported = true;
        return info;
    }
 
private:
    std::shared_ptr<MockTelephonyPermission> permission_;
};
 
/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_001
 * @tc.name: CheckPermission with all mask bits and all permissions granted
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_001,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::GET_NETWORK_INFO)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::CELL_LOCATION)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::MANAGE_CALL_FOR_DEVICES)).Times(AtLeast(2)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::SET_TELEPHONY_STATE)).Times(AtLeast(1)).WillRepeatedly(Return(true));
    uint32_t mask = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE |
        TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO | TelephonyObserverBroker::OBSERVER_MASK_CCALL_STATE |
        TelephonyObserverBroker::OBSERVER_MASK_VOIP_CALL_STATE |
        TelephonyObserverBroker::OBSERVER_MASK_SIM_ACTIVE_STATE;
    EXPECT_TRUE(service->CheckPermission(mask));
}
 
/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_002
 * @tc.name: CheckPermission with empty mask and no permission check
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_002,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).Times(0);
    EXPECT_TRUE(service->CheckPermission(0));
}
 
/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_003
 * @tc.name: CheckPermission with NETWORK_STATE bit and GET_NETWORK_INFO denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_003,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::GET_NETWORK_INFO)).WillRepeatedly(Return(false));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE));
}

/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_005
 * @tc.name: CheckPermission with CELL_INFO bit and CELL_LOCATION denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_005,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(Permission::CELL_LOCATION)).WillRepeatedly(Return(false));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO));
}
 
/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_006
 * @tc.name: CheckPermission with CCALL_STATE bit and MANAGE_CALL_FOR_DEVICES denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_006,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::MANAGE_CALL_FOR_DEVICES)).WillRepeatedly(Return(false));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_CCALL_STATE));
}
 
/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_007
 * @tc.name: CheckPermission with VOIP_CALL_STATE bit and MANAGE_CALL_FOR_DEVICES denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_007,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::MANAGE_CALL_FOR_DEVICES)).WillRepeatedly(Return(false));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_VOIP_CALL_STATE));
}
 
/**
 * @tc.number: TelephonyStateRegistryService_CheckPermission_008
 * @tc.name: CheckPermission with SIM_ACTIVE_STATE bit and SET_TELEPHONY_STATE denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_CheckPermission_008,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::SET_TELEPHONY_STATE)).WillRepeatedly(Return(false));
    EXPECT_FALSE(service->CheckPermission(TelephonyObserverBroker::OBSERVER_MASK_SIM_ACTIVE_STATE));
}
 
/**
 * @tc.number: TelephonyStateRegistryService_UpdateVoIPCallState_001
 * @tc.name: UpdateVoIPCallState with SET_TELEPHONY_STATE permission denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_UpdateVoIPCallState_001,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(
        Permission::SET_TELEPHONY_STATE)).WillRepeatedly(Return(false));
    auto result = service->UpdateVoIPCallState(BuildVoipInfo());
    EXPECT_EQ(result, TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED);
}
 
/**
 * @tc.number: TelephonyStateRegistryService_UpdateVoIPCallState_002
 * @tc.name: UpdateVoIPCallState with permission granted and no state record
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_UpdateVoIPCallState_002,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    auto result = service->UpdateVoIPCallState(BuildVoipInfo());
    EXPECT_EQ(result, TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
}
 
/**
 * @tc.number: TelephonyStateRegistryService_UpdateVoIPCallState_003
 * @tc.name: UpdateVoIPCallState with record not listening for voip state
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_UpdateVoIPCallState_003,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    sptr<VoipStateSpyObserver> observer = new VoipStateSpyObserver();
    TelephonyStateRegistryRecord record;
    record.mask_ = TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT;
    record.telephonyObserver_ = observer;
    service->stateRecords_.push_back(record);
    auto result = service->UpdateVoIPCallState(BuildVoipInfo());
    EXPECT_EQ(result, TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
    EXPECT_EQ(observer->voipCallbackCount_, 0);
}
 
/**
 * @tc.number: TelephonyStateRegistryService_UpdateVoIPCallState_004
 * @tc.name: UpdateVoIPCallState with voip listener record whose observer is null
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_UpdateVoIPCallState_004,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    TelephonyStateRegistryRecord record;
    record.mask_ = TelephonyObserverBroker::OBSERVER_MASK_VOIP_CALL_STATE;
    record.telephonyObserver_ = nullptr;
    service->stateRecords_.push_back(record);
    auto result = service->UpdateVoIPCallState(BuildVoipInfo());
    EXPECT_EQ(result, TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
}
 
/**
 * @tc.number: TelephonyStateRegistryService_UpdateVoIPCallState_005
 * @tc.name: UpdateVoIPCallState with observer lacking MANAGE_CALL_FOR_DEVICES permission
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_UpdateVoIPCallState_005,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    sptr<VoipStateSpyObserver> observer = new VoipStateSpyObserver();
    TelephonyStateRegistryRecord record;
    record.mask_ = TelephonyObserverBroker::OBSERVER_MASK_VOIP_CALL_STATE;
    record.telephonyObserver_ = observer;
    record.tokenId_ = UNREGISTERED_TOKEN_ID;
    service->stateRecords_.push_back(record);
    auto result = service->UpdateVoIPCallState(BuildVoipInfo());
    EXPECT_EQ(result, TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
    EXPECT_EQ(observer->voipCallbackCount_, 0);
}
 
/**
 * @tc.number: TelephonyStateRegistryService_UpdateVoIPCallState_006
 * @tc.name: UpdateVoIPCallState with voip listener granted MANAGE_CALL_FOR_DEVICES permission
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryServiceTest, TelephonyStateRegistryService_UpdateVoIPCallState_006,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    VoipStateDeviceToken deviceToken;
    ASSERT_TRUE(deviceToken.GetTokenId() != 0);
    sptr<VoipStateSpyObserver> observer = new VoipStateSpyObserver();
    TelephonyStateRegistryRecord record;
    record.mask_ = TelephonyObserverBroker::OBSERVER_MASK_VOIP_CALL_STATE;
    record.telephonyObserver_ = observer;
    record.tokenId_ = static_cast<int32_t>(deviceToken.GetTokenId());
    service->stateRecords_.push_back(record);
    auto result = service->UpdateVoIPCallState(BuildVoipInfo());
    EXPECT_EQ(result, TELEPHONY_SUCCESS);
    EXPECT_EQ(observer->voipCallbackCount_, 1);
    EXPECT_EQ(observer->lastVoipInfo_.appName, "com.example.voip");
    EXPECT_EQ(observer->lastVoipInfo_.contactName, "Bob");
    EXPECT_EQ(observer->lastVoipInfo_.callType, VoIPCallType::VIDEO_CONFERENCE);
    EXPECT_EQ(observer->lastVoipInfo_.callState, VoIPCallState::ANSWERED);
    EXPECT_TRUE(observer->lastVoipInfo_.isVoiceAnswerSupported);
}
} // namespace Telephony
} // namespace OHOS