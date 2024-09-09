/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <initializer_list>
#include <list>
#include <map>
#include <string>
#include <utility>

#include "cj_lambda.h"
#include "state_registry_errors.h"
#include "telephony_errors.h"
#include "telephony_observer_impl.h"

namespace OHOS {
namespace Telephony {
namespace {
const std::map<std::string_view, TelephonyUpdateEventType> eventMap {
    { "networkStateChange", TelephonyUpdateEventType::EVENT_NETWORK_STATE_UPDATE },
    { "callStateChange", TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE },
    { "signalInfoChange", TelephonyUpdateEventType::EVENT_SIGNAL_STRENGTHS_UPDATE },
    { "simStateChange", TelephonyUpdateEventType::EVENT_SIM_STATE_UPDATE },
    { "cellInfoChange", TelephonyUpdateEventType::EVENT_CELL_INFO_UPDATE },
    { "cellularDataConnectionStateChange", TelephonyUpdateEventType::EVENT_DATA_CONNECTION_UPDATE },
    { "cellularDataFlowChange", TelephonyUpdateEventType::EVENT_CELLULAR_DATA_FLOW_UPDATE },
    { "cfuIndicatorChange", TelephonyUpdateEventType::EVENT_CFU_INDICATOR_UPDATE },
    { "voiceMailMsgIndicatorChange", TelephonyUpdateEventType::EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE },
    { "iccAccountInfoChange", TelephonyUpdateEventType::EVENT_ICC_ACCOUNT_CHANGE },
};

TelephonyUpdateEventType GetEventType(std::string_view event)
{
    auto serched = eventMap.find(event);
    return (serched != eventMap.end() ? serched->second : TelephonyUpdateEventType::NONE_EVENT_TYPE);
}
} // namespace

static int32_t ConvertCJErrCode(int32_t errCode)
{
    switch (errCode) {
        case TELEPHONY_ERR_ARGUMENT_MISMATCH:
        case TELEPHONY_ERR_ARGUMENT_INVALID:
        case TELEPHONY_ERR_ARGUMENT_NULL:
        case TELEPHONY_ERR_SLOTID_INVALID:
        case ERROR_SLOT_ID_INVALID:
            // 83000001
            return CJ_ERROR_TELEPHONY_ARGUMENT_ERROR;
        case TELEPHONY_ERR_DESCRIPTOR_MISMATCH:
        case TELEPHONY_ERR_WRITE_DESCRIPTOR_TOKEN_FAIL:
        case TELEPHONY_ERR_WRITE_DATA_FAIL:
        case TELEPHONY_ERR_READ_DATA_FAIL:
        case TELEPHONY_ERR_WRITE_REPLY_FAIL:
        case TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL:
        case TELEPHONY_ERR_REGISTER_CALLBACK_FAIL:
        case TELEPHONY_ERR_CALLBACK_ALREADY_REGISTERED:
        case TELEPHONY_ERR_UNINIT:
        case TELEPHONY_ERR_UNREGISTER_CALLBACK_FAIL:
            // 83000002
            return CJ_ERROR_TELEPHONY_SERVICE_ERROR;
        case TELEPHONY_ERR_VCARD_FILE_INVALID:
        case TELEPHONY_ERR_FAIL:
        case TELEPHONY_ERR_MEMCPY_FAIL:
        case TELEPHONY_ERR_MEMSET_FAIL:
        case TELEPHONY_ERR_STRCPY_FAIL:
        case TELEPHONY_ERR_LOCAL_PTR_NULL:
        case TELEPHONY_ERR_SUBSCRIBE_BROADCAST_FAIL:
        case TELEPHONY_ERR_PUBLISH_BROADCAST_FAIL:
        case TELEPHONY_ERR_STRTOINT_FAIL:
        case TELEPHONY_ERR_ADD_DEATH_RECIPIENT_FAIL:
        case TELEPHONY_ERR_RIL_CMD_FAIL:
        case TELEPHONY_ERR_DATABASE_WRITE_FAIL:
        case TELEPHONY_ERR_DATABASE_READ_FAIL:
        case TELEPHONY_ERR_UNKNOWN_NETWORK_TYPE:
            // 83000003
            return CJ_ERROR_TELEPHONY_SYSTEM_ERROR;
        case TELEPHONY_ERR_NO_SIM_CARD:
            // 83000004
            return CJ_ERROR_TELEPHONY_NO_SIM_CARD;
        case TELEPHONY_ERR_AIRPLANE_MODE_ON:
            // 83000005
            return CJ_ERROR_TELEPHONY_AIRPLANE_MODE_ON;
        case TELEPHONY_ERR_NETWORK_NOT_IN_SERVICE:
            // 83000006
            return CJ_ERROR_TELEPHONY_NETWORK_NOT_IN_SERVICE;
        case TELEPHONY_ERR_PERMISSION_ERR:
            // 201
            return CJ_ERROR_TELEPHONY_PERMISSION_DENIED;
        case TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API:
            // 202
            return CJ_ERROR_TELEPHONY_PERMISSION_DENIED;
        default:
            return errCode;
    }
}

static bool IsValidSlotIdEx(TelephonyUpdateEventType eventType, int32_t slotId)
{
    int32_t defaultSlotId = DEFAULT_SIM_SLOT_ID;
    if (eventType == TelephonyUpdateEventType::EVENT_CALL_STATE_UPDATE) {
        defaultSlotId = -1;
    }
    // One more slot for VSim.
    return ((slotId >= defaultSlotId) && (slotId < SIM_SLOT_COUNT + 1));
}

static int32_t RegisterEventListener(EventListener &eventListener)
{
    auto handler = DelayedSingleton<ObserverEventHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return handler->RegisterEventListener(eventListener);
}

static int32_t UnregisterEventListener(const TelephonyUpdateEventType eventType, int64_t funcId)
{
    auto handler = DelayedSingleton<ObserverEventHandler>::GetInstance();
    if (handler == nullptr) {
        TELEPHONY_LOGE("Get event handler failed");
        return TELEPHONY_ERR_LOCAL_PTR_NULL;
    }
    return handler->UnregisterEventListener(eventType, funcId);
}

static void NativeOn(EventListener listener, int32_t &errCode)
{
    errCode = TELEPHONY_SUCCESS
    if (SIM_SLOT_COUNT == 0) {
        TELEPHONY_LOGE("The device is not support sim card.");
        return;
    }

    if (!IsValidSlotIdEx(listener.eventType, listener.slotId)) {
        TELEPHONY_LOGE("NativeOn slotId is invalid");
        errCode = ERROR_SLOT_ID_INVALID;
        return;
    }

    errCode = RegisterEventListener(listener);
}

static int32_t On(std::string_view event, ObserverOptions options, int64_t funcId)
{
    int32_t errCode = TELEPHONY_SUCCESS;
    std::shared_ptr<bool> isDeleting = std::make_shared<bool>(false);
    auto func = reinterpret_cast<void(*)(void*)>(funcId);
    auto callbackRef = CJLambda::Create(func);
    if (callbackRef == nullptr) {
        errCode = TELEPHONY_ERR_ARGUMENT_NULL;
        TELEPHONY_LOGE("TelephonyObserverImpl on register callback is nullptr.");
        return errCode;
    }

    EventListener listener {
        GetEventType(event),
        options.slotId,
        funcId,
        callbackRef,
        isDeleting
    };
    if (listener.eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        NativeOn(listener, errCode);
    } else {
        TELEPHONY_LOGE("TelephonyObserverImpl on register eventType is unkonw.");
        errCode = TELEPHONY_ERR_ARGUMENT_INVALID;
    }

    if (errCode == TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED) {
        errCode = TELEPHONY_ERR_PERMISSION_ERR;
    }
    return errCode;
}

static int32_t Off(std::string_view event, int64_t funcId = -1)
{
    int32_t errCode = TELEPHONY_SUCCESS;
    TelephonyUpdateEventType eventType = GetEventType(event);
    if (eventType != TelephonyUpdateEventType::NONE_EVENT_TYPE) {
        UnregisterEventListener(eventType, funcId);
    } else {
        errCode = TELEPHONY_ERR_ARGUMENT_INVALID;
    }
    if (errCode == TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED) {
        errCode = TELEPHONY_ERR_PERMISSION_ERR;
    }
    return errCode;
}

int32_t TelephonyObserverImpl::OnNetworkStateChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("networkStateChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffNetworkStateChange(int64_t funcId)
{
    std::string_view event("networkStateChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllNetworkStateChange()
{
    std::string_view event("networkStateChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OnSignalInfoChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("signalInfoChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffSignalInfoChange(int64_t funcId)
{
    std::string_view event("signalInfoChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllSignalInfoChange()
{
    std::string_view event("signalInfoChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OnCallStateChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("callStateChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffCallStateChange(int64_t funcId)
{
    std::string_view event("callStateChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllCallStateChange()
{
    std::string_view event("callStateChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OnCellularDataConnectionStateChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("cellularDataConnectionStateChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffCellularDataConnectionStateChange(int64_t funcId)
{
    std::string_view event("cellularDataConnectionStateChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllCellularDataConnectionStateChange()
{
    std::string_view event("cellularDataConnectionStateChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OnCellularDataFlowChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("cellularDataFlowChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffCellularDataFlowChange(int64_t funcId)
{
    std::string_view event("cellularDataFlowChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllCellularDataFlowChange()
{
    std::string_view event("cellularDataFlowChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OnSimStateChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("simStateChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffSimStateChange(int64_t funcId)
{
    std::string_view event("simStateChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllSimStateChange()
{
    std::string_view event("simStateChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OnIccAccountInfoChange(ObserverOptions options, int64_t funcId)
{
    std::string_view event("iccAccountInfoChange");
    int32_t errCode = On(event, options, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffIccAccountInfoChange(int64_t funcId)
{
    std::string_view event("iccAccountInfoChange");
    int32_t errCode = Off(event, funcId);
    return ConvertCJErrCode(errCode);
}

int32_t TelephonyObserverImpl::OffAllIccAccountInfoChange()
{
    std::string_view event("iccAccountInfoChange");
    int32_t errCode = Off(event);
    return ConvertCJErrCode(errCode);
}

// FfiTelephonyObserver
void FfiTelephonyObserver::OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &phoneNumber)
{
    TELEPHONY_LOGI("OnCallStateUpdated slotId = %{public}d, callState = %{public}d", slotId, callState);
    std::unique_ptr<CallStateUpdateInfo> callStateInfo =
        std::make_unique<CallStateUpdateInfo>(slotId, callState, phoneNumber);
    if (callStateInfo == nullptr) {
        TELEPHONY_LOGE("callStateInfo is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_CALL_STATE_UPDATE),
        callStateInfo);
}

void FfiTelephonyObserver::OnSignalInfoUpdated(
    int32_t slotId, const std::vector<sptr<SignalInformation>> &signalInfoList)
{
    TELEPHONY_LOGI("OnSignalInfoUpdated slotId = %{public}d, signalInfoList.size = %{public}zu", slotId,
        signalInfoList.size());
    std::unique_ptr<SignalUpdateInfo> infoList = std::make_unique<SignalUpdateInfo>(slotId, signalInfoList);
    if (infoList == nullptr) {
        TELEPHONY_LOGE("SignalUpdateInfo is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_SIGNAL_INFO_UPDATE),
        infoList);
}

void FfiTelephonyObserver::OnNetworkStateUpdated(int32_t slotId, const sptr<NetworkState> &networkState)
{
    TELEPHONY_LOGI(
        "OnNetworkStateUpdated slotId = %{public}d, networkState = %{public}d", slotId, networkState == nullptr);
    std::unique_ptr<NetworkStateUpdateInfo> networkStateUpdateInfo =
        std::make_unique<NetworkStateUpdateInfo>(slotId, networkState);
    if (networkStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("NetworkStateUpdateInfo is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_NETWORK_STATE_UPDATE),
        networkStateUpdateInfo);
}

void FfiTelephonyObserver::OnSimStateUpdated(
    int32_t slotId, CardType type, SimState state, LockReason reason)
{
    TELEPHONY_LOGI("OnSimStateUpdated slotId = %{public}d, simState =  %{public}d", slotId, state);
    std::unique_ptr<SimStateUpdateInfo> simStateUpdateInfo =
        std::make_unique<SimStateUpdateInfo>(slotId, type, state, reason);
    if (simStateUpdateInfo == nullptr) {
        TELEPHONY_LOGE("SimStateUpdateInfo is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_SIM_STATE_UPDATE),
        simStateUpdateInfo);
}

void FfiTelephonyObserver::OnCellInfoUpdated(int32_t slotId, const std::vector<sptr<CellInformation>> &vec)
{
    TELEPHONY_LOGI("OnCellInfoUpdated slotId = %{public}d, cell info size =  %{public}zu", slotId, vec.size());
    std::unique_ptr<CellInfomationUpdate> cellInfo = std::make_unique<CellInfomationUpdate>(slotId, vec);
    if (cellInfo == nullptr) {
        TELEPHONY_LOGE("CellInfomationUpdate is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_CELL_INFOMATION_UPDATE),
        cellInfo);
}

void FfiTelephonyObserver::OnCellularDataConnectStateUpdated(
    int32_t slotId, int32_t dataState, int32_t networkType)
{
    TELEPHONY_LOGD("OnCellularDataConnectStateUpdated slotId=%{public}d, dataState=%{public}d, networkType="
        "%{public}d",
        slotId, dataState, networkType);
    std::unique_ptr<CellularDataConnectState> cellularDataConnectState =
        std::make_unique<CellularDataConnectState>(slotId, dataState, networkType);
    if (cellularDataConnectState == nullptr) {
        TELEPHONY_LOGE("OnCellularDataConnectStateUpdated cellularDataConnectState is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE),
        cellularDataConnectState);
}

void FfiTelephonyObserver::OnCellularDataFlowUpdated(int32_t slotId, int32_t dataFlowType)
{
    TELEPHONY_LOGI(
        "OnCellularDataFlowUpdated slotId = %{public}d, dataFlowType =  %{public}d", slotId, dataFlowType);
    std::unique_ptr<CellularDataFlowUpdate> cellularDataFlowUpdateInfo =
        std::make_unique<CellularDataFlowUpdate>(slotId, dataFlowType);
    if (cellularDataFlowUpdateInfo == nullptr) {
        TELEPHONY_LOGE("CellularDataFlowUpdate is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_CELLULAR_DATA_FLOW_UPDATE),
        cellularDataFlowUpdateInfo);
}

void FfiTelephonyObserver::OnCfuIndicatorUpdated(int32_t slotId, bool cfuResult)
{
    TELEPHONY_LOGI("OnCfuIndicatorUpdated slotId = %{public}d, cfuResult = %{public}d", slotId, cfuResult);
    std::unique_ptr<CfuIndicatorUpdate> cfuIndicatorUpdateInfo =
        std::make_unique<CfuIndicatorUpdate>(slotId, cfuResult);
    if (cfuIndicatorUpdateInfo == nullptr) {
        TELEPHONY_LOGE("CfuIndicatorUpdate is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_CFU_INDICATOR_UPDATE),
        cfuIndicatorUpdateInfo);
}

void FfiTelephonyObserver::OnIccAccountUpdated()
{
    TELEPHONY_LOGI("OnIccAccountUpdated begin");
    TelephonyObserverImpl::SendEvent(static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_ICC_ACCOUNT_UPDATE));
}

void FfiTelephonyObserver::OnVoiceMailMsgIndicatorUpdated(int32_t slotId, bool voiceMailMsgResult)
{
    TELEPHONY_LOGI("OnVoiceMailMsgIndicatorUpdated slotId = %{public}d, voiceMailMsgResult =  %{public}d", slotId,
        voiceMailMsgResult);
    std::unique_ptr<VoiceMailMsgIndicatorUpdate> voiceMailMsgIndicatorUpdateInfo =
        std::make_unique<VoiceMailMsgIndicatorUpdate>(slotId, voiceMailMsgResult);
    if (voiceMailMsgIndicatorUpdateInfo == nullptr) {
        TELEPHONY_LOGE("VoiceMailMsgIndicatorUpdate is nullptr!");
        return;
    }
    TelephonyObserverImpl::SendEvent(
        static_cast<uint32_t>(TelephonyCallbackEventId::EVENT_ON_VOICE_MAIL_MSG_INDICATOR_UPDATE),
        voiceMailMsgIndicatorUpdateInfo);
}
}
}