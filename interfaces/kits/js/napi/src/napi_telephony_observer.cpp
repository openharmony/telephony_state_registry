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

#include "napi_telephony_observer.h"

#include "telephony_callback_event_id.h"
#include "update_infos.h"
#include "event_listener_manager.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void NapiTelephonyObserver::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber)
{
    TELEPHONY_LOGD("OnCallStateUpdated callState = %{public}d", callState);
    auto callStateInfo = std::make_unique<CallStateUpdateInfo>(callState, phoneNumber);
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE), callStateInfo);
}

void NapiTelephonyObserver::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &signalInfoList)
{
    TELEPHONY_LOGD("OnSignalInfoUpdated signalInfoList.size = %{public}zu", signalInfoList.size());
    auto infoList = std::make_unique<SignalUpdateInfo>(signalInfoList);
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE), infoList);
}

void NapiTelephonyObserver::OnNetworkStateUpdated(const sptr<NetworkState> &networkState)
{
    TELEPHONY_LOGD("OnNetworkStateUpdated networkState == null %{public}d", networkState == nullptr);
    auto networkStateUpdateInfo = std::make_unique<NetworkStateUpdateInfo>(networkState);
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE), networkStateUpdateInfo);
}

void NapiTelephonyObserver::OnSimStateUpdated(int32_t state, const std::u16string &reason)
{
    TELEPHONY_LOGD("OnSimStateUpdated simState =  %{public}d", state);
    auto simStateUpdateInfo = std::make_unique<SimStateUpdateInfo>(state, reason);
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE), simStateUpdateInfo);
}
} // namespace Telephony
} // namespace OHOS
