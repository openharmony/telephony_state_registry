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

#ifndef STATE_REGISTER_TEST_H
#define STATE_REGISTER_TEST_H

#include <iostream>
#include <list>
#include <securec.h>

#include "accesstoken_kit.h"
#include "gtest/gtest.h"
#include "i_telephony_state_notify.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "state_registry_errors.h"
#include "state_registry_observer.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "telephony_types.h"
#include "token_setproc.h"

namespace OHOS {
namespace Telephony {
using namespace Security::AccessToken;
using namespace testing::ext;
using Security::AccessToken::AccessTokenID;

HapInfoParams testStateRegistryParams = {
    .bundleName = "tel_state_registry_test",
    .userID = 1,
    .instIndex = 0,
    .appIDDesc = "test",
};

PermissionDef testNetPermGetNetworkInfoDef = {
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .bundleName = "tel_state_registry_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test state registry",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testNetPermGetNetworkInfo = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.GET_NETWORK_INFO",
    .resDeviceID = { "local" },
};

PermissionDef testNetPermSetTelephonyStateDef = {
    .permissionName = "ohos.permission.SET_TELEPHONY_STATE",
    .bundleName = "tel_state_registry_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test state registry",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testNetSetTelephonyState = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.SET_TELEPHONY_STATE",
    .resDeviceID = { "local" },
};

PermissionDef testNetPermLocationDef = {
    .permissionName = "ohos.permission.LOCATION",
    .bundleName = "tel_state_registry_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test state registry",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testNetPermLocation = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.LOCATION",
    .resDeviceID = { "local" },
};

PermissionDef testNetPermReadCallLogDef = {
    .permissionName = "ohos.permission.READ_CALL_LOG",
    .bundleName = "tel_state_registry_test",
    .grantMode = 1, // SYSTEM_GRANT
    .label = "label",
    .labelId = 1,
    .description = "Test state registry",
    .descriptionId = 1,
    .availableLevel = APL_SYSTEM_BASIC,
};

PermissionStateFull testNetPermReadCallLog = {
    .grantFlags = { 2 }, // PERMISSION_USER_SET
    .grantStatus = { PermissionState::PERMISSION_GRANTED },
    .isGeneral = true,
    .permissionName = "ohos.permission.READ_CALL_LOG",
    .resDeviceID = { "local" },
};

HapPolicyParams testPolicyParams = {
    .apl = APL_SYSTEM_BASIC,
    .domain = "test.domain",
    .permList = { testNetPermGetNetworkInfoDef, testNetPermSetTelephonyStateDef, testNetPermLocationDef,
        testNetPermReadCallLogDef },
    .permStateList = { testNetPermGetNetworkInfo, testNetSetTelephonyState, testNetPermLocation,
        testNetPermReadCallLog },
};

class AccessToken {
public:
    AccessToken()
    {
        currentID_ = GetSelfTokenID();
        AccessTokenIDEx tokenIdEx = AccessTokenKit::AllocHapToken(testStateRegistryParams, testPolicyParams);
        accessID_ = tokenIdEx.tokenIdExStruct.tokenID;
        SetSelfTokenID(accessID_);
    }
    ~AccessToken()
    {
        AccessTokenKit::DeleteToken(accessID_);
        SetSelfTokenID(currentID_);
    }

private:
    AccessTokenID currentID_ = 0;
    AccessTokenID accessID_ = 0;
};

class StateRegistryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void CreateProxy();
    static bool HasSimCard(int32_t slotId);

    void UpdateCallState(int32_t slotId);
    void UpdateCallStateForSlotId(int32_t slotId);
    void UpdateSignalInfo(int32_t slotId);
    void UpdateCellularDataConnectState(int32_t slotId);
    void UpdateCellularDataFlow(int32_t slotId);
    void UpdateSimState(int32_t slotId);
    void UpdateNetworkState(int32_t slotId);

public:
    using RequestFuncType = void (StateRegistryTest::*)(int32_t slotId);
    std::map<char, RequestFuncType> requestFuncMap_;

private:
    static void InitTelephonyObserver();
    static void DisableTelephonyObserver();

private:
    static sptr<StateRegistryObserver> telephonyObserver0_;
    static sptr<StateRegistryObserver> telephonyObserver1_;
    TelephonyObserver telephonyObserver;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_REGISTER_TEST_H