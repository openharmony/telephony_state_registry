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

#include "telephony_state_registry_stub.h"

#include "ipc_skeleton.h"
#include "string_ex.h"

#include "sim_state_type.h"
#include "state_registry_errors.h"
#include "telephony_permission.h"

namespace OHOS {
namespace Telephony {
TelephonyStateRegistryStub::TelephonyStateRegistryStub()
{
    memberFuncMap_[StateNotifyInterfaceCode::CELL_INFO] = &TelephonyStateRegistryStub::OnUpdateCellInfo;
    memberFuncMap_[StateNotifyInterfaceCode::SIM_STATE] = &TelephonyStateRegistryStub::OnUpdateSimState;
    memberFuncMap_[StateNotifyInterfaceCode::SIGNAL_INFO] = &TelephonyStateRegistryStub::OnUpdateSignalInfo;
    memberFuncMap_[StateNotifyInterfaceCode::NET_WORK_STATE] = &TelephonyStateRegistryStub::OnUpdateNetworkState;
    memberFuncMap_[StateNotifyInterfaceCode::CALL_STATE] = &TelephonyStateRegistryStub::OnUpdateCallState;
    memberFuncMap_[StateNotifyInterfaceCode::CALL_STATE_FOR_ID] =
        &TelephonyStateRegistryStub::OnUpdateCallStateForSlotId;
    memberFuncMap_[StateNotifyInterfaceCode::CELLULAR_DATA_STATE] =
        &TelephonyStateRegistryStub::OnUpdateCellularDataConnectState;
    memberFuncMap_[StateNotifyInterfaceCode::CELLULAR_DATA_FLOW] =
        &TelephonyStateRegistryStub::OnUpdateCellularDataFlow;
    memberFuncMap_[StateNotifyInterfaceCode::ADD_OBSERVER] = &TelephonyStateRegistryStub::OnRegisterStateChange;
    memberFuncMap_[StateNotifyInterfaceCode::REMOVE_OBSERVER] = &TelephonyStateRegistryStub::OnUnregisterStateChange;
    memberFuncMap_[StateNotifyInterfaceCode::CFU_INDICATOR] = &TelephonyStateRegistryStub::OnUpdateCfuIndicator;
    memberFuncMap_[StateNotifyInterfaceCode::VOICE_MAIL_MSG_INDICATOR] =
        &TelephonyStateRegistryStub::OnUpdateVoiceMailMsgIndicator;
    memberFuncMap_[StateNotifyInterfaceCode::ICC_ACCOUNT_CHANGE] = &TelephonyStateRegistryStub::OnIccAccountUpdated;
}

TelephonyStateRegistryStub::~TelephonyStateRegistryStub()
{
    memberFuncMap_.clear();
}

int32_t TelephonyStateRegistryStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TELEPHONY_LOGD("TelephonyStateRegistryStub::OnRemoteRequest start##code = %{public}u", code);
    std::u16string myToken = TelephonyStateRegistryStub::GetDescriptor();
    std::u16string remoteToken = data.ReadInterfaceToken();
    if (myToken != remoteToken) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnRemoteRequest end##descriptor checked fail");
        return TELEPHONY_ERR_DESCRIPTOR_MISMATCH;
    }
    auto itFunc = memberFuncMap_.find(static_cast<StateNotifyInterfaceCode>(code));
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnRemoteRequest end##ret=%{public}d", ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateCallState(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t callState = data.ReadInt32();
    std::u16string phoneNumber = data.ReadString16();
    int32_t ret = UpdateCallState(slotId, callState, phoneNumber);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCallState end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateSimState(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    CardType type = static_cast<CardType>(data.ReadInt32());
    SimState state = static_cast<SimState>(data.ReadInt32());
    LockReason reason = static_cast<LockReason>(data.ReadInt32());
    int32_t ret = UpdateSimState(slotId, type, state, reason);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateSimState end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateCallStateForSlotId(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t callId = data.ReadInt32();
    int32_t callState = data.ReadInt32();
    std::u16string incomingNumber = data.ReadString16();
    int32_t ret = UpdateCallStateForSlotId(slotId, callId, callState, incomingNumber);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCallStateForSlotId end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateCellularDataConnectState(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t dataState = data.ReadInt32();
    int32_t networkType = data.ReadInt32();
    int32_t ret = UpdateCellularDataConnectState(slotId, dataState, networkType);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateCellularDataConnectState end fail##ret=%{public}d", ret);
    }
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateCellularDataFlow(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t flowData = data.ReadInt32();
    int32_t ret = UpdateCellularDataFlow(slotId, flowData);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateCellularDataFlow end fail##ret=%{public}d", ret);
    }
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateSignalInfo(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t slotId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateSignalInfo size=%{public}d", size);
    size = ((size > SignalInformation::MAX_SIGNAL_NUM) ? 0 : size);
    if (size <= 0) {
        ret = TELEPHONY_ERR_FAIL;
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateSignalInfo size <= 0");
        return ret;
    }
    std::vector<sptr<SignalInformation>> result;
    parseSignalInfos(data, size, result);
    ret = UpdateSignalInfo(slotId, result);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateSignalInfo end fail##ret=%{public}d", ret);
    }
    reply.WriteInt32(ret);
    return ret;
}

void TelephonyStateRegistryStub::parseSignalInfos(
    MessageParcel &data, const int32_t size, std::vector<sptr<SignalInformation>> &result)
{
    SignalInformation::NetworkType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<SignalInformation::NetworkType>(data.ReadInt32());
        switch (type) {
            case SignalInformation::NetworkType::GSM: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::GSM");
                std::unique_ptr<GsmSignalInformation> signal = std::make_unique<GsmSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::CDMA: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::CDMA");
                std::unique_ptr<CdmaSignalInformation> signal = std::make_unique<CdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::LTE:
                [[fallthrough]]; // fall_through
            case SignalInformation::NetworkType::NR: {
                ParseLteNrSignalInfos(data, result, type);
                break;
            }
            case SignalInformation::NetworkType::WCDMA: {
                TELEPHONY_LOGI("TelephonyStateRegistryStub::parseSignalInfos NetworkType::Wcdma");
                std::unique_ptr<WcdmaSignalInformation> signal = std::make_unique<WcdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            default:
                break;
        }
    }
}

void TelephonyStateRegistryStub::ParseLteNrSignalInfos(
    MessageParcel &data, std::vector<sptr<SignalInformation>> &result, SignalInformation::NetworkType type)
{
    switch (type) {
        case SignalInformation::NetworkType::LTE: {
            TELEPHONY_LOGI("TelephonyStateRegistryStub::ParseLteNrSignalInfos NetworkType::LTE");
            std::unique_ptr<LteSignalInformation> signal = std::make_unique<LteSignalInformation>();
            if (signal != nullptr) {
                signal->ReadFromParcel(data);
                result.emplace_back(signal.release());
            }
            break;
        }
        case SignalInformation::NetworkType::NR: {
            TELEPHONY_LOGI("TelephonyStateRegistryStub::ParseSignalInfos");
            std::unique_ptr<NrSignalInformation> signal = std::make_unique<NrSignalInformation>();
            if (signal != nullptr) {
                signal->ReadFromParcel(data);
                result.emplace_back(signal.release());
            }
            break;
        }
        default:
            break;
    }
}

int32_t TelephonyStateRegistryStub::OnUpdateCellInfo(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t slotId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGI("TelephonyStateRegistryStub OnUpdateCellInfo:size=%{public}d", size);
    size = ((size > CellInformation::MAX_CELL_NUM) ? 0 : size);
    if (size <= 0) {
        ret = TELEPHONY_ERR_FAIL;
        TELEPHONY_LOGE("TelephonyStateRegistryStub the size less than or equal to 0!");
        return ret;
    }
    std::vector<sptr<CellInformation>> cells;
    CellInformation::CellType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<CellInformation::CellType>(data.ReadInt32());
        switch (type) {
            case CellInformation::CellType::CELL_TYPE_GSM: {
                std::unique_ptr<GsmCellInformation> cell = std::make_unique<GsmCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            case CellInformation::CellType::CELL_TYPE_LTE: {
                std::unique_ptr<LteCellInformation> cell = std::make_unique<LteCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            case CellInformation::CellType::CELL_TYPE_NR: {
                std::unique_ptr<NrCellInformation> cell = std::make_unique<NrCellInformation>();
                if (cell != nullptr) {
                    cell->ReadFromParcel(data);
                    cells.emplace_back(cell.release());
                }
                break;
            }
            default:
                break;
        }
    }
    ret = UpdateCellInfo(slotId, cells);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCellInfo end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateNetworkState(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t slotId = data.ReadInt32();
    sptr<NetworkState> result = NetworkState::Unmarshalling(data);
    if (result == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUpdateNetworkState GetNetworkStatus  is null");
        ret = TELEPHONY_ERR_FAIL;
        return ret;
    }
    ret = UpdateNetworkState(slotId, result);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateNetworkState end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnRegisterStateChange(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t slotId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    bool notifyNow = data.ReadBool();
    sptr<TelephonyObserverBroker> callback = nullptr;
    ret = ReadData(data, reply, callback);
    if (ret != TELEPHONY_SUCCESS) {
        reply.WriteInt32(ret);
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnRegisterStateChange ReadData failed");
        return ret;
    }
    ret = RegisterStateChange(callback, slotId, mask, notifyNow);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnRegisterStateChange end fail##ret=%{public}d", ret);
    }
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUnregisterStateChange(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    int32_t ret = UnregisterStateChange(slotId, mask);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnUnregisterStateChange end fail##ret=%{public}d", ret);
    }
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::ReadData(
    MessageParcel &data, MessageParcel &reply, sptr<TelephonyObserverBroker> &callback)
{
    int32_t result = TELEPHONY_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::ReadData  remote is nullptr.");
        result = TELEPHONY_ERR_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    callback = iface_cast<TelephonyObserverBroker>(remote);
    if (callback == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::ReadData callback is nullptr.");
        result = TELEPHONY_ERR_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    return result;
}

int32_t TelephonyStateRegistryStub::OnUpdateCfuIndicator(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    bool cfuResult = data.ReadBool();
    int32_t ret = UpdateCfuIndicator(slotId, cfuResult);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateCfuIndicator end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnUpdateVoiceMailMsgIndicator(MessageParcel &data, MessageParcel &reply)
{
    int32_t slotId = data.ReadInt32();
    bool voiceMailMsgResult = data.ReadBool();
    int32_t ret = UpdateVoiceMailMsgIndicator(slotId, voiceMailMsgResult);
    TELEPHONY_LOGI("TelephonyStateRegistryStub::OnUpdateVoiceMailMsgIndicator end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::OnIccAccountUpdated(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = UpdateIccAccount();
    TELEPHONY_LOGI("end##ret=%{public}d", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t slotId, uint32_t mask, bool isUpdate)
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    std::string bundleName = "";
    TelephonyPermission::GetBundleNameByUid(uid, bundleName);
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    return RegisterStateChange(telephonyObserver, slotId, mask, bundleName, isUpdate,
        IPCSkeleton::GetCallingPid(), uid, tokenId);
}

int32_t TelephonyStateRegistryStub::UnregisterStateChange(int32_t slotId, uint32_t mask)
{
    int32_t tokenId = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID());
    return UnregisterStateChange(slotId, mask, tokenId, IPCSkeleton::GetCallingPid());
}
} // namespace Telephony
} // namespace OHOS