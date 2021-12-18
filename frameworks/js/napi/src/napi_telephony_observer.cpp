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
#include "event_listener_manager.h"
#include "telephony_callback_event_id.h"
#include "telephony_log_wrapper.h"
#include "update_infos.h"

namespace OHOS {
namespace Telephony {
void NapiTelephonyObserver::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber)
{
    TELEPHONY_LOGI("OnCallStateUpdated callState = %{public}d, phoneNumber = %{public}s", callState,
        NapiUtil::ToUtf8(phoneNumber).c_str());
    std::unique_ptr<CallStateUpdateInfo> callStateInfo =
        std::make_unique<CallStateUpdateInfo>(callState, phoneNumber);
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE), callStateInfo);
}

void NapiTelephonyObserver::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &signalInfoList)
{
    TELEPHONY_LOGI("OnSignalInfoUpdated signalInfoList.size = %{public}zu", signalInfoList.size());
    std::unique_ptr<SignalUpdateInfo> infoList = std::make_unique<SignalUpdateInfo>(signalInfoList);
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE), infoList);
}

void NapiTelephonyObserver::OnNetworkStateUpdated(const sptr<NetworkState> &networkState)
{
    TELEPHONY_LOGI("OnNetworkStateUpdated networkState = %{public}d", networkState == nullptr);
    std::unique_ptr<NetworkStateUpdateInfo> networkStateUpdateInfo =
        std::make_unique<NetworkStateUpdateInfo>(networkState);
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE), networkStateUpdateInfo);
}

void NapiTelephonyObserver::OnSimStateUpdated(SimState state, LockReason reason)
{
    TELEPHONY_LOGI("OnSimStateUpdated simState =  %{public}d", state);
    std::unique_ptr<SimStateUpdateInfo> simStateUpdateInfo = std::make_unique<SimStateUpdateInfo>(state, reason);
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE), simStateUpdateInfo);
}

void NapiTelephonyObserver::OnCellInfoUpdated(const std::vector<sptr<CellInformation>> &vec)
{
    TELEPHONY_LOGI("OnCellInfoUpdated cell info size =  %{public}zu", vec.size());
    std::unique_ptr<CellInfomationUpdate> cellInfo = std::make_unique<CellInfomationUpdate>(vec);
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_CELL_INFOMATION_UPDATE), cellInfo);
}
} // namespace Telephony
} // namespace OHOS