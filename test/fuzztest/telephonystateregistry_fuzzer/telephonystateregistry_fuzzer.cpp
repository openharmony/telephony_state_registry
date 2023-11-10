/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "telephonystateregistry_fuzzer.h"

#include <cstddef>
#include <cstdint>
#define private public
#define protected public
#include "addstateregistrytoken_fuzzer.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "telephony_observer.h"
#include "telephony_state_registry_service.h"
#include "telephony_state_registry_stub.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t BOOL_NUM = 2;
constexpr int32_t SLOT_NUM = 2;
constexpr int32_t ROAMING_NUM = 4;
constexpr int32_t REG_NUM = 6;
constexpr int32_t CELL_NUM = 7;
constexpr int32_t SIGNAL_NUM = 6;
constexpr int32_t SIGNAL_PLUS = 1;
constexpr int32_t NR_NUM = 7;
constexpr int32_t RADIO_NUM = 13;

bool IsServiceInited()
{
    if (!g_isInited) {
        DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnStart();
        if (DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->GetServiceRunningState() ==
            static_cast<int32_t>(ServiceRunningState::STATE_RUNNING)) {
            g_isInited = true;
        }
    }
    return g_isInited;
}

void OnRemoteRequest(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    MessageParcel dataMessageParcel;
    if (!dataMessageParcel.WriteInterfaceToken(TelephonyStateRegistryStub::GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(size);
    MessageParcel reply;
    MessageOption option;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnRemoteRequest(
        code, dataMessageParcel, reply, option);
}

void CreateGsmCellInfo(std::unique_ptr<GsmCellInformation> &cell, const uint8_t *data, size_t size)
{
    if (cell == nullptr) {
        return;
    }
    cell->lac_ = static_cast<int32_t>(size);
    cell->bsic_ = static_cast<int32_t>(size);
    cell->arfcn_ = static_cast<int32_t>(size);
    std::string mcc(reinterpret_cast<const char *>(data), size);
    cell->mcc_ = mcc;
    std::string mnc(reinterpret_cast<const char *>(data), size);
    cell->mnc_ = mnc;
    cell->cellId_ = static_cast<int32_t>(size);
    cell->timeStamp_ = static_cast<uint64_t>(size);
    cell->signalIntensity_ = static_cast<int32_t>(size);
    cell->signalLevel_ = static_cast<int32_t>(size);
    cell->isCamped_ = static_cast<int32_t>(size % BOOL_NUM);
}

void CreateLteCellInfo(std::unique_ptr<LteCellInformation> &cell, const uint8_t *data, size_t size)
{
    if (cell == nullptr) {
        return;
    }
    cell->pci_ = static_cast<int32_t>(size);
    cell->tac_ = static_cast<int32_t>(size);
    cell->earfcn_ = static_cast<int32_t>(size);
    std::string mcc(reinterpret_cast<const char *>(data), size);
    cell->mcc_ = mcc;
    std::string mnc(reinterpret_cast<const char *>(data), size);
    cell->mnc_ = mnc;
    cell->cellId_ = static_cast<int32_t>(size);
    cell->timeStamp_ = static_cast<uint64_t>(size);
    cell->signalIntensity_ = static_cast<int32_t>(size);
    cell->signalLevel_ = static_cast<int32_t>(size);
    cell->isCamped_ = static_cast<int32_t>(size % BOOL_NUM);
}

void UpdateCellInfo(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    int32_t loopSize = static_cast<int32_t>(size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(loopSize);
    for (int32_t i = 0; i < loopSize; i++) {
        CellInformation::CellType type = static_cast<CellInformation::CellType>(size % CELL_NUM);
        if (type == CellInformation::CellType::CELL_TYPE_GSM) {
            std::unique_ptr<GsmCellInformation> cell = std::make_unique<GsmCellInformation>();
            if (cell == nullptr) {
                return;
            }
            CreateGsmCellInfo(cell, data, size);
            cell->Marshalling(dataMessageParcel);
        }
        if (type == CellInformation::CellType::CELL_TYPE_LTE) {
            std::unique_ptr<LteCellInformation> cell = std::make_unique<LteCellInformation>();
            if (cell == nullptr) {
                return;
            }
            CreateLteCellInfo(cell, data, size);
            cell->Marshalling(dataMessageParcel);
        }
    }
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateCellInfo(dataMessageParcel, reply);
}

void UpdateCallState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    int32_t callState = static_cast<int32_t>(size);
    std::string phoneNumber(reinterpret_cast<const char *>(data), size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(callState);
    dataMessageParcel.WriteString16(Str8ToStr16(phoneNumber));
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateCallState(dataMessageParcel, reply);
}

void UpdateCallStateForSlotId(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    int32_t callId = static_cast<int32_t>(size);
    int32_t callState = static_cast<int32_t>(size);
    std::string incomingNumber(reinterpret_cast<const char *>(data), size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(callId);
    dataMessageParcel.WriteInt32(callState);
    dataMessageParcel.WriteString16(Str8ToStr16(incomingNumber));
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateCallStateForSlotId(
        dataMessageParcel, reply);
}

void CreateGsmSignalInfo(std::unique_ptr<GsmSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    signal->signalBar_ = static_cast<int32_t>(size);
    signal->gsmRxlev_ = static_cast<int32_t>(size);
    signal->gsmBer_ = static_cast<int32_t>(size);
}

void CreateCDMASignalInfo(std::unique_ptr<CdmaSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    signal->signalBar_ = static_cast<int32_t>(size);
    signal->cdmaRssi_ = static_cast<int32_t>(size);
    signal->cdmaEcno_ = static_cast<int32_t>(size);
}

void CreateLTESignalInfo(std::unique_ptr<LteSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    signal->signalBar_ = static_cast<int32_t>(size);
    signal->rxlev_ = static_cast<int32_t>(size);
    signal->lteRsrp_ = static_cast<int32_t>(size);
    signal->lteRsrq_ = static_cast<int32_t>(size);
    signal->lteSnr_ = static_cast<int32_t>(size);
}

void CreateWCDMASignalInfo(std::unique_ptr<WcdmaSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    signal->signalBar_ = static_cast<int32_t>(size);
    signal->wcdmaRxlev_ = static_cast<int32_t>(size);
    signal->wcdmaRscp_ = static_cast<int32_t>(size);
    signal->wcdmaEcio_ = static_cast<int32_t>(size);
    signal->wcdmaBer_ = static_cast<int32_t>(size);
}

void CreateNRSignalInfo(std::unique_ptr<NrSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    signal->signalBar_ = static_cast<int32_t>(size);
    signal->nrRsrp_ = static_cast<int32_t>(size);
    signal->nrRsrq_ = static_cast<int32_t>(size);
    signal->nrSinr_ = static_cast<int32_t>(size);
}

void UpdateLteNrSignalInfo(const uint8_t *data, size_t size, MessageParcel &dataMessageParcel,
    SignalInformation::NetworkType type)
{
    if (type == SignalInformation::NetworkType::LTE) {
        std::unique_ptr<LteSignalInformation> signal = std::make_unique<LteSignalInformation>();
        if (signal == nullptr) {
            return;
        }
        CreateLTESignalInfo(signal, data, size);
        signal->Marshalling(dataMessageParcel);
    }
    if (type == SignalInformation::NetworkType::NR) {
        std::unique_ptr<NrSignalInformation> signal = std::make_unique<NrSignalInformation>();
        if (signal == nullptr) {
            return;
        }
        CreateNRSignalInfo(signal, data, size);
        signal->Marshalling(dataMessageParcel);
    }
}

void UpdateSignalInfo(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    int32_t loopSize = static_cast<int32_t>(size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(loopSize);
    for (int32_t i = 0; i < loopSize; i++) {
        SignalInformation::NetworkType type =
            static_cast<SignalInformation::NetworkType>(size % SIGNAL_NUM + SIGNAL_PLUS);
        if (type == SignalInformation::NetworkType::GSM) {
            std::unique_ptr<GsmSignalInformation> signal = std::make_unique<GsmSignalInformation>();
            if (signal == nullptr) {
                return;
            }
            CreateGsmSignalInfo(signal, data, size);
            signal->Marshalling(dataMessageParcel);
        }
        if (type == SignalInformation::NetworkType::CDMA) {
            std::unique_ptr<CdmaSignalInformation> signal = std::make_unique<CdmaSignalInformation>();
            if (signal == nullptr) {
                return;
            }
            CreateCDMASignalInfo(signal, data, size);
            signal->Marshalling(dataMessageParcel);
        }
        if (type == SignalInformation::NetworkType::LTE || type == SignalInformation::NetworkType::NR) {
            UpdateLteNrSignalInfo(data, size, dataMessageParcel, type);
        }
        if (type == SignalInformation::NetworkType::WCDMA) {
            std::unique_ptr<WcdmaSignalInformation> signal = std::make_unique<WcdmaSignalInformation>();
            if (signal == nullptr) {
                return;
            }
            CreateWCDMASignalInfo(signal, data, size);
            signal->Marshalling(dataMessageParcel);
        }
    }
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateSignalInfo(dataMessageParcel, reply);
}

void UpdateNetworkState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    auto networkState = std::make_unique<NetworkState>();
    if (networkState == nullptr) {
        return;
    }
    networkState->isEmergency_ = static_cast<int32_t>(size % BOOL_NUM);
    std::string mOperatorNumeric(reinterpret_cast<const char *>(data), size);
    std::string mFullName(reinterpret_cast<const char *>(data), size);
    std::string mShortName(reinterpret_cast<const char *>(data), size);
    networkState->psOperatorInfo_.operatorNumeric = mOperatorNumeric;
    networkState->psOperatorInfo_.fullName = mFullName;
    networkState->psOperatorInfo_.shortName = mShortName;
    networkState->csOperatorInfo_.operatorNumeric = mOperatorNumeric;
    networkState->csOperatorInfo_.fullName = mFullName;
    networkState->csOperatorInfo_.shortName = mShortName;
    networkState->csRoaming_ = static_cast<RoamingType>(size % ROAMING_NUM);
    networkState->psRoaming_ = static_cast<RoamingType>(size % ROAMING_NUM);
    networkState->psRegStatus_ = static_cast<RegServiceState>(size % REG_NUM);
    networkState->csRegStatus_ = static_cast<RegServiceState>(size % REG_NUM);
    networkState->psRadioTech_ = static_cast<RadioTech>(size % RADIO_NUM);
    networkState->lastPsRadioTech_ = static_cast<RadioTech>(size % RADIO_NUM);
    networkState->lastCfgTech_ = static_cast<RadioTech>(size % RADIO_NUM);
    networkState->csRadioTech_ = static_cast<RadioTech>(size % RADIO_NUM);
    networkState->cfgTech_ = static_cast<RadioTech>(size % RADIO_NUM);
    networkState->nrState_ = static_cast<NrState>(size % NR_NUM);
    networkState->Marshalling(dataMessageParcel);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateNetworkState(dataMessageParcel, reply);
}

void UpdateCellularDataConnectState(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    int32_t dataState = static_cast<int32_t>(size);
    int32_t networkType = static_cast<int32_t>(size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(dataState);
    dataMessageParcel.WriteInt32(networkType);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateCellularDataConnectState(
        dataMessageParcel, reply);
}

void UpdateCellularDataFlow(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    int32_t flowData = static_cast<int32_t>(size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteInt32(flowData);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateCellularDataFlow(dataMessageParcel, reply);
}

void UpdateCfuIndicator(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    bool cfuResult = static_cast<bool>(size % BOOL_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBool(cfuResult);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateCfuIndicator(dataMessageParcel, reply);
}

void UpdateVoiceMailMsgIndicator(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    bool voiceMailMsgResult = static_cast<bool>(size % BOOL_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.WriteBool(voiceMailMsgResult);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    DelayedSingleton<TelephonyStateRegistryService>::GetInstance()->OnUpdateVoiceMailMsgIndicator(
        dataMessageParcel, reply);
}

void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    OnRemoteRequest(data, size);
    UpdateCellInfo(data, size);
    UpdateCallState(data, size);
    UpdateCallStateForSlotId(data, size);
    UpdateSignalInfo(data, size);
    UpdateNetworkState(data, size);
    UpdateCellularDataConnectState(data, size);
    UpdateCellularDataFlow(data, size);
    UpdateCfuIndicator(data, size);
    UpdateVoiceMailMsgIndicator(data, size);
    return;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AddStateRegistryTokenFuzzer token;
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

