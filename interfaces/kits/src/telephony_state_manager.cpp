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
#include "telephony_state_manager.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "hilog_wrapper.h"
#include "telephony_errors.h"

namespace OHOS {
namespace TelephonyState {
TelephonyStateManager::TelephonyStateManager()
{
    ConnectService();
}

TelephonyStateManager::~TelephonyStateManager() {}

int32_t TelephonyStateManager::AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t subId, uint32_t mask, const std::u16string &callingPackage, bool notifyNow)
{
    int32_t result = TELEPHONY_NO_ERROR;
    if (telephonyStateNotify_ != nullptr) {
        result =
            telephonyStateNotify_->RegisterStateChange(telephonyObserver, subId, mask, callingPackage, notifyNow);
        return result;
    }
    return result;
}

int32_t TelephonyStateManager::RemoveStateObserver(int32_t subId, uint32_t mask)
{
    int32_t result = TELEPHONY_NO_ERROR;
    if (telephonyStateNotify_ != nullptr) {
        result = telephonyStateNotify_->UnregisterStateChange(subId, mask);
        return result;
    }
    return result;
}

int TelephonyStateManager::ConnectService()
{
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("TelephonyStateManager::ConnectService() GetSystemAbilityManager() null\n");
        return TELEPHONY_FAIL;
    }

    sptr<IRemoteObject> object = systemManager->GetSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);

    if (object != nullptr) {
        HILOG_ERROR("TelephonyStateManager::ConnectService() IRemoteObject not null\n");
        telephonyStateNotify_ = iface_cast<TelephonyState::ITelephonyStateNotify>(object);
    }

    if (telephonyStateNotify_ == nullptr) {
        HILOG_ERROR("TelephonyStateManager::ConnectService() telephonyStateNotify_ null\n");
        return TELEPHONY_FAIL;
    }
    HILOG_DEBUG("TelephonyStateManager::ConnectService() success\n");
    return TELEPHONY_NO_ERROR;
}
} // namespace TelephonyState
} // namespace OHOS