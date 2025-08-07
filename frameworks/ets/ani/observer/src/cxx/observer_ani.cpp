/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <memory>
#include "observer_ani.h"
#include "telephony_log_wrapper.h"
#include "telephony_errors.h"
#include "telephony_state_manager.h"
#include "telephony_types.h"
#include "state_registry_errors.h"
#include "napi_util.h"
#include "cxx.h"
#include "wrapper.rs.h"

namespace OHOS {
using namespace Telephony;
namespace ObserverAni {

constexpr const char *OBSERVER_JS_PERMISSION_ERROR_STRING =
    "Permission denied. An attempt was made to Observer "
    "On forbidden by permission : ohos.permission.GET_NETWORK_INFO or ohos.permission.LOCATION ";

void AniTelephonyObserver::OnSignalInfoUpdated(
    int32_t slotId, const std::vector<sptr<Telephony::SignalInformation>> &signalInfoList)
{
    rust::Vec<SignalInformationAni> signalInfoListAni;
    for (auto infoItem : signalInfoList) {
        int32_t signalType = static_cast<int32_t>(infoItem->GetNetworkType());
        int32_t signalLevel = infoItem->GetSignalLevel();
        int32_t dBm = infoItem->GetSignalIntensity();
        SignalInformationAni info {
            .signal_type = signalType,
            .signal_level = signalLevel,
            .d_bm = dBm,
        };
        signalInfoListAni.push_back(info);
    }
    on_signal_info_updated(slotId, signalInfoListAni);
}

void AniTelephonyObserver::OnNetworkStateUpdated(int32_t slotId, const sptr<Telephony::NetworkState> &networkState)
{
    std::string longOperatorName = networkState->GetLongOperatorName();
    std::string shortOperatorName = networkState->GetShortOperatorName();
    std::string plmnNumeric = networkState->GetPlmnNumeric();
    bool isRoaming = networkState->IsRoaming();
    int32_t regStatus = static_cast<int32_t>(networkState->GetRegStatus());
    bool isEmergency = networkState->IsEmergency();
    int32_t cfgTech = static_cast<int32_t>(networkState->GetCfgTech());
    int32_t nsaState = static_cast<int32_t>(networkState->GetNrState());

    NetworkStateAni info {
        .long_operator_name = rust::String(longOperatorName),
        .short_operator_name = rust::String(shortOperatorName),
        .plmn_numeric = rust::String(plmnNumeric),
        .is_roaming = isRoaming,
        .reg_state = regStatus,
        .cfg_tech = cfgTech,
        .nsa_state = nsaState,
        .is_emergency = isEmergency,
    };
    on_network_state_updated(slotId, info);
}

void AniTelephonyObserver::OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &num)
{
    std::string phone_number = NapiUtil::ToUtf8(num);
    CallStateAni info{
        .state = callState,
        .num = rust::String(phone_number),
    };
    on_call_state_updated(slotId, info);
}

void AniTelephonyObserver::OnSimStateUpdated(
    int32_t slotId, Telephony::CardType type, Telephony::SimState state, Telephony::LockReason reason)
{
    int32_t cardType = static_cast<int32_t>(type);
    int32_t simState = static_cast<int32_t>(state);
    int32_t lockReason = static_cast<int32_t>(reason);
    on_sim_state_updated(slotId, cardType, simState, lockReason);
}

void AniTelephonyObserver::OnCellInfoUpdated(int32_t slotId, const std::vector<sptr<Telephony::CellInformation>> &vec)
{
    rust::Vec<CellInformationAni> cellInfoListAni;
    for (auto info_item : vec) {
        int32_t networkType = static_cast<int32_t>(info_item->GetNetworkType());
        int32_t signalLevel = info_item->GetSignalLevel();
        int32_t dBm = info_item->GetSignalIntensity();

        SignalInformationAni signalInfo {
            .signal_type = networkType,
            .signal_level = signalLevel,
            .d_bm = dBm,
        };
        CellInformationAni cellInfo {
            .network_type = networkType,
            .signal_information = signalInfo,
        };
        cellInfoListAni.push_back(cellInfo);
    }
    on_cell_info_updated(slotId, cellInfoListAni);
}

void AniTelephonyObserver::OnCellularDataConnectStateUpdated(
    int32_t slotId, int32_t dataState, int32_t networkType)
{
    on_cellular_data_connect_state_updated(slotId, dataState, networkType);
}

void AniTelephonyObserver::OnCellularDataFlowUpdated(int32_t slotId, int32_t dataFlowType)
{
    on_cellular_data_flow_updated(slotId, dataFlowType);
}

void AniTelephonyObserver::OnIccAccountUpdated()
{
    on_icc_account_updated();
}

static inline ArktsError ConvertArktsError(int32_t errorCode)
{
    JsError error = NapiUtil::ConverErrorMessageForJs(errorCode);

    ArktsError ArktsErr = {
        .errorCode = error.errorCode,
        .errorMessage = rust::string(error.errorMessage),
    };
    return ArktsErr;
}

bool IsValidSlotIdEx(int32_t slotId, uint32_t eventType)
{
    int32_t defaultSlotId = DEFAULT_SIM_SLOT_ID;
    if (eventType == static_cast<uint32_t>(TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE)) {
        defaultSlotId = -1;
    }
    // One more slot for VSim.
    return ((slotId >= defaultSlotId) && (slotId < SIM_SLOT_COUNT + 1));
}

ArktsError EventListenerRegister(int32_t slotId, uint32_t eventType)
{
    int32_t errorCode;

    AniTelephonyObserver *telephonyObserver = std::make_unique<AniTelephonyObserver>().release();
    sptr<Telephony::TelephonyObserverBroker> observer(telephonyObserver);
    if (observer == nullptr) {
        TELEPHONY_LOGE("error by observer nullptr");
        errorCode = Telephony::TELEPHONY_ERR_LOCAL_PTR_NULL;
        return ConvertArktsError(errorCode);
    }
    errorCode = Telephony::TelephonyStateManager::AddStateObserver(
        observer, slotId, eventType,
        eventType == static_cast<uint32_t>(TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE));
    if (errorCode == TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED) {
        ArktsError ArktsErr = {
            .errorCode = JS_ERROR_TELEPHONY_PERMISSION_DENIED,
            .errorMessage = rust::string(OBSERVER_JS_PERMISSION_ERROR_STRING),
        };
        return ArktsErr;
    }
    
    return ConvertArktsError(errorCode);
}

ArktsError EventListenerUnRegister(int32_t slotId, uint32_t eventType)
{
    int32_t errorCode = TelephonyStateManager::RemoveStateObserver(slotId, eventType);
    if (errorCode == TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED) {
        ArktsError ArktsErr = {
            .errorCode = JS_ERROR_TELEPHONY_PERMISSION_DENIED,
            .errorMessage = rust::string(OBSERVER_JS_PERMISSION_ERROR_STRING),
        };
        return ArktsErr;
    }
    return ConvertArktsError(errorCode);
}

} //namespace ObserverAni
} //namespace OHOS