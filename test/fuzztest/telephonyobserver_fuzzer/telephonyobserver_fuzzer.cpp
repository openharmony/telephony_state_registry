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
#include "telephonyobserver_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include "addstateregistrytoken_fuzzer.h"
#define private public
#include "securec.h"
#include "system_ability_definition.h"
#include "telephony_observer.h"
#include "telephony_state_registry_service.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t BOOL_NUM = 2;
constexpr int32_t SLOT_NUM = 2;
constexpr int32_t ROAMING_NUM = 4;
constexpr int32_t REG_NUM = 6;
constexpr int32_t NR_NUM = 7;
constexpr int32_t RADIO_NUM = 13;
TelephonyObserver telephonyObserver;

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
    if (!dataMessageParcel.WriteInterfaceToken(TelephonyObserver::GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    uint32_t code = static_cast<uint32_t>(size);
    MessageParcel reply;
    MessageOption option;
    telephonyObserver.OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void CallStateUpdatedInner(const uint8_t *data, size_t size)
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
    telephonyObserver.OnCallStateUpdatedInner(dataMessageParcel, reply);
}

void SignalInfoUpdatedInner(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    telephonyObserver.OnSignalInfoUpdatedInner(dataMessageParcel, reply);
}

void NetworkStateUpdatedInner(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    auto networkState = std::make_shared<NetworkState>();
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
    telephonyObserver.OnNetworkStateUpdatedInner(dataMessageParcel, reply);
}

void CellInfoUpdatedInner(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = static_cast<int32_t>(size % SLOT_NUM);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    dataMessageParcel.RewindRead(0);
    MessageParcel reply;
    telephonyObserver.OnCellInfoUpdatedInner(dataMessageParcel, reply);
}

void DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    OnRemoteRequest(data, size);
    CallStateUpdatedInner(data, size);
    SignalInfoUpdatedInner(data, size);
    NetworkStateUpdatedInner(data, size);
    CellInfoUpdatedInner(data, size);
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
