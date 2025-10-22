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
#include <fuzzer/FuzzedDataProvider.h>
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "securec.h"
#include "state_registry_ipc_interface_code.h"
#include "system_ability_definition.h"
#include "telephony_observer.h"
#include "telephony_state_manager.h"
#include "telephony_state_registry_service.h"
#include "telephony_state_registry_stub.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t ROAMING_NUM = 4;
constexpr int32_t REG_NUM = 6;
constexpr int32_t CELL_NUM = 7;
constexpr int32_t SIGNAL_NUM = 6;
constexpr int32_t SIGNAL_PLUS = 1;
constexpr int32_t NR_NUM = 7;
constexpr int32_t RADIO_NUM = 13;
constexpr int32_t MIN_SLOT_ID = -1;
constexpr int32_t MAX_SLOT_ID = 4;
constexpr int32_t MAX_LOOP_SIZE = 1000;

int32_t GetRandomInt(int min, int max, const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    return fdp.ConsumeIntegralInRange<int32_t>(min, max);
}

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
    int32_t maxCode = static_cast<int32_t>(StateNotifyInterfaceCode::ICC_ACCOUNT_CHANGE) + 1;
    uint32_t code = static_cast<uint32_t>(GetRandomInt(0, maxCode, data, size));
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
    cell->lac_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->bsic_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->arfcn_ = GetRandomInt(0, INT32_MAX, data, size);
    std::string mcc(reinterpret_cast<const char *>(data), size);
    cell->mcc_ = mcc;
    std::string mnc(reinterpret_cast<const char *>(data), size);
    cell->mnc_ = mnc;
    cell->cellId_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->timeStamp_ = static_cast<uint64_t>(GetRandomInt(0, INT32_MAX, data, size));
    cell->signalIntensity_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->signalLevel_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->isCamped_ = GetRandomInt(0, 1, data, size);
}

void CreateLteCellInfo(std::unique_ptr<LteCellInformation> &cell, const uint8_t *data, size_t size)
{
    if (cell == nullptr) {
        return;
    }
    cell->pci_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->tac_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->earfcn_ = GetRandomInt(0, INT32_MAX, data, size);
    std::string mcc(reinterpret_cast<const char *>(data), size);
    cell->mcc_ = mcc;
    std::string mnc(reinterpret_cast<const char *>(data), size);
    cell->mnc_ = mnc;
    cell->cellId_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->timeStamp_ = static_cast<uint64_t>(GetRandomInt(0, INT32_MAX, data, size));
    cell->signalIntensity_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->signalLevel_ = GetRandomInt(0, INT32_MAX, data, size);
    cell->isCamped_ = GetRandomInt(0, 1, data, size);
}

void UpdateCellInfo(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    int32_t loopSize = GetRandomInt(0, MAX_LOOP_SIZE, data, size);
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
    int32_t maxState = static_cast<int32_t>(Telephony::CallStatus::CALL_STATUS_ANSWERED) + 1;
    int32_t callState = GetRandomInt(-1, maxState, data, size);
    std::string phoneNumber(reinterpret_cast<const char *>(data), size);
    MessageParcel dataMessageParcel;
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    int32_t maxState = static_cast<int32_t>(Telephony::CallStatus::CALL_STATUS_ANSWERED) + 1;
    int32_t callState = GetRandomInt(-1, maxState, data, size);
    std::string incomingNumber(reinterpret_cast<const char *>(data), size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
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
    FuzzedDataProvider provider(data, size);
    signal->signalBar_ = provider.ConsumeIntegral<int32_t>();
    signal->gsmRxlev_ = provider.ConsumeIntegral<int32_t>();
    signal->gsmBer_ = provider.ConsumeIntegral<int32_t>();
}

void CreateCDMASignalInfo(std::unique_ptr<CdmaSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    FuzzedDataProvider provider(data, size);
    signal->signalBar_ = provider.ConsumeIntegral<int32_t>();

    signal->cdmaRssi_ = provider.ConsumeIntegral<int32_t>();

    signal->cdmaEcno_ = provider.ConsumeIntegral<int32_t>();
}

void CreateLTESignalInfo(std::unique_ptr<LteSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    FuzzedDataProvider provider(data, size);
    signal->signalBar_ = provider.ConsumeIntegral<int32_t>();

    signal->rxlev_ = provider.ConsumeIntegral<int32_t>();

    signal->lteRsrp_ = provider.ConsumeIntegral<int32_t>();

    signal->lteRsrq_ = provider.ConsumeIntegral<int32_t>();

    signal->lteSnr_ = provider.ConsumeIntegral<int32_t>();
}

void CreateWCDMASignalInfo(std::unique_ptr<WcdmaSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    FuzzedDataProvider provider(data, size);
    signal->signalBar_ = provider.ConsumeIntegral<int32_t>();

    signal->wcdmaRxlev_ = provider.ConsumeIntegral<int32_t>();

    signal->wcdmaRscp_ = provider.ConsumeIntegral<int32_t>();

    signal->wcdmaEcio_ = provider.ConsumeIntegral<int32_t>();

    signal->wcdmaBer_ = provider.ConsumeIntegral<int32_t>();
}

void CreateNRSignalInfo(std::unique_ptr<NrSignalInformation> &signal, const uint8_t *data, size_t size)
{
    if (signal == nullptr) {
        return;
    }
    FuzzedDataProvider provider(data, size);
    signal->signalBar_ = provider.ConsumeIntegral<int32_t>();

    signal->nrRsrp_ = provider.ConsumeIntegral<int32_t>();

    signal->nrRsrq_ = provider.ConsumeIntegral<int32_t>();

    signal->nrSinr_ = provider.ConsumeIntegral<int32_t>();
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    int32_t loopSize = GetRandomInt(0, MAX_LOOP_SIZE, data, size);
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    auto networkState = std::make_unique<NetworkState>();
    if (networkState == nullptr) {
        return;
    }
    networkState->isEmergency_ = static_cast<bool>(GetRandomInt(0, 1, data, size));
    std::string mOperatorNumeric(reinterpret_cast<const char *>(data), size);
    std::string mFullName(reinterpret_cast<const char *>(data), size);
    std::string mShortName(reinterpret_cast<const char *>(data), size);
    networkState->psOperatorInfo_.operatorNumeric = mOperatorNumeric;
    networkState->psOperatorInfo_.fullName = mFullName;
    networkState->psOperatorInfo_.shortName = mShortName;
    networkState->csOperatorInfo_.operatorNumeric = mOperatorNumeric;
    networkState->csOperatorInfo_.fullName = mFullName;
    networkState->csOperatorInfo_.shortName = mShortName;
    networkState->csRoaming_ = static_cast<RoamingType>(GetRandomInt(0, ROAMING_NUM, data, size));
    networkState->psRoaming_ = static_cast<RoamingType>(GetRandomInt(0, ROAMING_NUM, data, size));
    networkState->psRegStatus_ = static_cast<RegServiceState>(GetRandomInt(0, REG_NUM, data, size));
    networkState->csRegStatus_ = static_cast<RegServiceState>(GetRandomInt(0, REG_NUM, data, size));
    networkState->psRadioTech_ = static_cast<RadioTech>(GetRandomInt(0, RADIO_NUM, data, size));
    networkState->lastPsRadioTech_ = static_cast<RadioTech>(GetRandomInt(0, RADIO_NUM, data, size));
    networkState->lastCfgTech_ = static_cast<RadioTech>(GetRandomInt(0, RADIO_NUM, data, size));
    networkState->csRadioTech_ = static_cast<RadioTech>(GetRandomInt(0, RADIO_NUM, data, size));
    networkState->cfgTech_ = static_cast<RadioTech>(GetRandomInt(0, RADIO_NUM, data, size));
    networkState->nrState_ = static_cast<NrState>(GetRandomInt(0, NR_NUM, data, size));
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    int32_t offset = 0;
    int32_t dataState = static_cast<int32_t>(*data + offset);
    offset += sizeof(int32_t);
    int32_t networkType = static_cast<int32_t>(*data + offset);
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    int32_t flowData = GetRandomInt(0, INT32_MAX, data, size);
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    bool cfuResult = static_cast<bool>(GetRandomInt(0, 1, data, size));
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    bool voiceMailMsgResult = static_cast<bool>(GetRandomInt(0, 1, data, size));
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

