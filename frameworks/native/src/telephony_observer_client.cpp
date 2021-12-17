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

#include "telephony_observer_client.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "service_interface_death_recipient.h"
#include "system_ability_definition.h"
#include "telephony_log_wrapper.h"
#include "telephony_napi_common_error.h"

namespace OHOS {
namespace Telephony {
bool TelephonyObserverClient::InitStateObserverClient()
{
    if (telephonyStateNotify_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            TELEPHONY_LOGE(" Get system ability mgr failed.");
            return false;
        }
        sptr<IRemoteObject> remoteObject =
            systemAbilityManager->GetSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
        if (!remoteObject) {
            TELEPHONY_LOGE("Get Telephony State Register Service Failed.");
            return false;
        }
        telephonyStateNotify_ = iface_cast<ITelephonyStateNotify>(remoteObject);
        if ((!telephonyStateNotify_) || (!telephonyStateNotify_->AsObject())) {
            TELEPHONY_LOGE("Get Telephony State Register Proxy Failed.");
            return false;
        }
        recipient_ = new ServiceInterfaceDeathRecipient<TelephonyObserverClient>();
        if (recipient_ == nullptr) {
            TELEPHONY_LOGE("Failed to create death Recipient ptr of telephony state registry!");
            return false;
        }
        telephonyStateNotify_->AsObject()->AddDeathRecipient(recipient_);
    }
    TELEPHONY_LOGE("Get Telephony State Register Service Success.");
    return true;
}

void TelephonyObserverClient::ResetServiceProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((telephonyStateNotify_ != nullptr) && (telephonyStateNotify_->AsObject() != nullptr)) {
        telephonyStateNotify_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    telephonyStateNotify_ = nullptr;
}

int32_t TelephonyObserverClient::AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t subId, uint32_t mask, const std::u16string &callingPackage, bool notifyNow)
{
    if (InitStateObserverClient()) {
        return telephonyStateNotify_->RegisterStateChange(
            telephonyObserver, subId, mask, callingPackage, notifyNow);
    }
    return ERROR_SERVICE_UNAVAILABLE;
}

int32_t TelephonyObserverClient::RemoveStateObserver(int32_t subId, uint32_t mask)
{
    if (InitStateObserverClient()) {
        return telephonyStateNotify_->UnregisterStateChange(subId, mask);
    }
    return ERROR_SERVICE_UNAVAILABLE;
}
} // namespace Telephony
} // namespace OHOS