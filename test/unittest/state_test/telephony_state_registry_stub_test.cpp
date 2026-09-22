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
 
#include "gmock/gmock.h"
#include "mock_telephony_permission.h"
#include "state_registry_errors.h"
#include "telephony_observer_broker.h"
#include "telephony_permission.h"
#include "telephony_state_registry_service.h"
#include "voip_call_state_info.h"
 
namespace OHOS {
namespace Telephony {
using namespace testing;
using namespace testing::ext;
 
namespace {
constexpr int32_t INVALID_CALL_TYPE_BELOW = -1;
constexpr int32_t INVALID_CALL_TYPE_ABOVE = 4;
constexpr int32_t INVALID_CALL_STATE_BELOW = -1;
constexpr int32_t INVALID_CALL_STATE_ABOVE = 9;
} // namespace
 
class TelephonyStateRegistryStubTest : public testing::Test {
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
    void WriteVoipCallStateParcel(MessageParcel &data, int32_t callType, int32_t callState, bool isVoiceAnswerSupported)
    {
        data.WriteString("com.example.voip");
        data.WriteString("Alice");
        data.WriteInt32(callType);
        data.WriteInt32(callState);
        data.WriteBool(isVoiceAnswerSupported);
    }
 
private:
    std::shared_ptr<MockTelephonyPermission> permission_;
};
 
/**
 * @tc.number: TelephonyStateRegistryStub_OnUpdateVoIPCallState_001
 * @tc.name: OnUpdateVoIPCallState with callType below enum lower bound
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryStubTest, TelephonyStateRegistryStub_OnUpdateVoIPCallState_001,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    MessageParcel data;
    MessageParcel reply;
    WriteVoipCallStateParcel(data, INVALID_CALL_TYPE_BELOW, static_cast<int32_t>(VoIPCallState::IDLE), true);
    auto result = service->OnUpdateVoIPCallState(data, reply);
    EXPECT_EQ(result, TELEPHONY_ERR_ARGUMENT_INVALID);
    EXPECT_EQ(reply.ReadInt32(), TELEPHONY_ERR_ARGUMENT_INVALID);
}
 
/**
 * @tc.number: TelephonyStateRegistryStub_OnUpdateVoIPCallState_002
 * @tc.name: OnUpdateVoIPCallState with callType above enum upper bound
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryStubTest, TelephonyStateRegistryStub_OnUpdateVoIPCallState_002,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    MessageParcel data;
    MessageParcel reply;
    WriteVoipCallStateParcel(data, INVALID_CALL_TYPE_ABOVE, static_cast<int32_t>(VoIPCallState::INCOMING), true);
    auto result = service->OnUpdateVoIPCallState(data, reply);
    EXPECT_EQ(result, TELEPHONY_ERR_ARGUMENT_INVALID);
    EXPECT_EQ(reply.ReadInt32(), TELEPHONY_ERR_ARGUMENT_INVALID);
}
 
/**
 * @tc.number: TelephonyStateRegistryStub_OnUpdateVoIPCallState_003
 * @tc.name: OnUpdateVoIPCallState with callState below enum lower bound
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryStubTest, TelephonyStateRegistryStub_OnUpdateVoIPCallState_003,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    MessageParcel data;
    MessageParcel reply;
    WriteVoipCallStateParcel(data, static_cast<int32_t>(VoIPCallType::VOICE), INVALID_CALL_STATE_BELOW, true);
    auto result = service->OnUpdateVoIPCallState(data, reply);
    EXPECT_EQ(result, TELEPHONY_ERR_ARGUMENT_INVALID);
    EXPECT_EQ(reply.ReadInt32(), TELEPHONY_ERR_ARGUMENT_INVALID);
}
 
/**
 * @tc.number: TelephonyStateRegistryStub_OnUpdateVoIPCallState_004
 * @tc.name: OnUpdateVoIPCallState with callState above enum upper bound
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryStubTest, TelephonyStateRegistryStub_OnUpdateVoIPCallState_004,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    MessageParcel data;
    MessageParcel reply;
    WriteVoipCallStateParcel(data, static_cast<int32_t>(VoIPCallType::VIDEO), INVALID_CALL_STATE_ABOVE, true);
    auto result = service->OnUpdateVoIPCallState(data, reply);
    EXPECT_EQ(result, TELEPHONY_ERR_ARGUMENT_INVALID);
    EXPECT_EQ(reply.ReadInt32(), TELEPHONY_ERR_ARGUMENT_INVALID);
}
 
/**
 * @tc.number: TelephonyStateRegistryStub_OnUpdateVoIPCallState_005
 * @tc.name: OnUpdateVoIPCallState with valid lower-bound enums and permission granted
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryStubTest, TelephonyStateRegistryStub_OnUpdateVoIPCallState_005,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(_)).WillRepeatedly(Return(true));
    MessageParcel data;
    MessageParcel reply;
    WriteVoipCallStateParcel(data, static_cast<int32_t>(VoIPCallType::VOICE),
        static_cast<int32_t>(VoIPCallState::IDLE), false);
    auto result = service->OnUpdateVoIPCallState(data, reply);
    EXPECT_EQ(result, NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST);
}
 
/**
 * @tc.number: TelephonyStateRegistryStub_OnUpdateVoIPCallState_006
 * @tc.name: OnUpdateVoIPCallState with valid upper-bound enums and permission denied
 * @tc.desc: Function test
 */
HWTEST_F(TelephonyStateRegistryStubTest, TelephonyStateRegistryStub_OnUpdateVoIPCallState_006,
    Function | MediumTest | Level0)
{
    auto service = DelayedSingleton<TelephonyStateRegistryService>::GetInstance();
    ASSERT_NE(service, nullptr);
    ASSERT_TRUE(permission_ != nullptr);
    service->stateRecords_.clear();
    EXPECT_CALL(*permission_, CheckPermission(Permission::SET_TELEPHONY_STATE)).WillRepeatedly(Return(false));
    MessageParcel data;
    MessageParcel reply;
    WriteVoipCallStateParcel(data, static_cast<int32_t>(VoIPCallType::VIDEO),
        static_cast<int32_t>(VoIPCallState::DISCONNECTED), true);
    auto result = service->OnUpdateVoIPCallState(data, reply);
    EXPECT_EQ(result, NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED);
}
} // namespace Telephony
} // namespace OHOS
