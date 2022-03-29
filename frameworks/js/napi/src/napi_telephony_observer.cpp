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
void NapiTelephonyObserver::OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &phoneNumber)
{
    TELEPHONY_LOGI("OnCallStateUpdated slotId = %{public}d, callState = %{public}d", slotId, callState);
    std::unique_ptr<CallStateUpdateInfo> callStateInfo =
        std::make_unique<CallStateUpdateInfo>(slotId, callState, phoneNumber);
    if (callStateInfo == nullptr) {
        TELEPHONY_LOGE("callStateInfo is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE), callStateInfo);
}

void NapiTelephonyObserver::OnSignalInfoUpdated(
    int32_t slotId, const std::vector<sptr<SignalInformation>> &signalInfoList)
{
    TELEPHONY_LOGI("OnSignalInfoUpdated slotId = %{public}d, signalInfoList.size = %{public}zu", slotId,
        signalInfoList.size());
    std::unique_ptr<SignalUpdateInfo> infoList = std::make_unique<SignalUpdateInfo>(slotId, signalInfoList);
    if (infoList == nullptr) {
        TELEPHONY_LOGE("SignalUpdateInfo is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE), infoList);
}

void NapiTelephonyObserver::OnNetworkStateUpdated(int32_t slotId, const sptr<NetworkState> &networkState)
{
    TELEPHONY_LOGI(
        "OnNetworkStateUpdated slotId = %{public}d, networkState = %{public}d", slotId, networkState == nullptr);
    std::unique_ptr<NetworkStateUpdateInfo> networkStateUpdateInfo =
        std::make_unique<NetworkStateUpdateInfo>(slotId, networkState);
    if (networkStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("NetworkStateUpdateInfo is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE), networkStateUpdateInfo);
}

void NapiTelephonyObserver::OnSimStateUpdated(
    int32_t slotId, CardType type, SimState state, LockReason reason)
{
    TELEPHONY_LOGI("OnSimStateUpdated slotId = %{public}d, simState =  %{public}d", slotId, state);
    std::unique_ptr<SimStateUpdateInfo> simStateUpdateInfo =
        std::make_unique<SimStateUpdateInfo>(slotId, type, state, reason);
    if (simStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("SimStateUpdateInfo is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE), simStateUpdateInfo);
}

void NapiTelephonyObserver::OnCellInfoUpdated(int32_t slotId, const std::vector<sptr<CellInformation>> &vec)
{
    TELEPHONY_LOGI("OnCellInfoUpdated slotId = %{public}d, cell info size =  %{public}zu", slotId, vec.size());
    std::unique_ptr<CellInfomationUpdate> cellInfo = std::make_unique<CellInfomationUpdate>(slotId, vec);
    if (cellInfo == nullptr) {
        TELEPHONY_LOGE("CellInfomationUpdate is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(ToUint32t(TelephonyCallbackEventId::EVENT_ON_CELL_INFOMATION_UPDATE), cellInfo);
}

void NapiTelephonyObserver::OnCellularDataConnectStateUpdated(
    int32_t slotId, int32_t dataState, int32_t networkType)
{
    TELEPHONY_LOGI(
        "OnCellularDataConnectStateUpdated slotId=%{public}d, dataState=%{public}d, networkType="
        "%{public}d",
        slotId, dataState, networkType);
    std::unique_ptr<CellularDataConnectState> cellularDataConnectState =
        std::make_unique<CellularDataConnectState>(slotId, dataState, networkType);
    if (cellularDataConnectState == nullptr) {
        TELEPHONY_LOGE("OnCellularDataConnectStateUpdated cellularDataConnectState is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE), cellularDataConnectState);
}

void NapiTelephonyObserver::OnCellularDataFlowUpdated(int32_t slotId, int32_t dataFlowType)
{
    TELEPHONY_LOGI(
        "OnCellularDataFlowUpdated slotId = %{public}d, dataFlowType =  %{public}d", slotId, dataFlowType);
    std::unique_ptr<CellularDataFlowUpdate> cellularDataFlowUpdateInfo =
        std::make_unique<CellularDataFlowUpdate>(slotId, dataFlowType);
    if (cellularDataFlowUpdateInfo == nullptr) {
        TELEPHONY_LOGE("CellularDataFlowUpdate is nullptr!");
        return;
    }
    EventListenerManager::SendEvent(
        ToUint32t(TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_FLOW_UPDATE), cellularDataFlowUpdateInfo);
}
} // namespace Telephony
} // namespace OHOS
