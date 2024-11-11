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
#include <fuzzer/FuzzedDataProvider.h>
#define private public
#include "securec.h"
#include "system_ability_definition.h"
#include "telephony_observer.h"
#include "telephony_observer_broker.h"
#include "telephony_state_manager.h"
#include "telephony_state_registry_service.h"

using namespace OHOS::Telephony;
namespace OHOS {
static bool g_isInited = false;
constexpr int32_t ROAMING_NUM = 4;
constexpr int32_t REG_NUM = 6;
constexpr int32_t NR_NUM = 7;
constexpr int32_t RADIO_NUM = 13;
constexpr int32_t MIN_SLOT_ID = -1;
constexpr int32_t MAX_SLOT_ID = 4;
TelephonyObserver telephonyObserver;

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
    if (!dataMessageParcel.WriteInterfaceToken(TelephonyObserver::GetDescriptor())) {
        return;
    }
    dataMessageParcel.WriteBuffer(data, size);
    dataMessageParcel.RewindRead(0);
    int32_t maxCode = static_cast<int32_t>(TelephonyObserver::ObserverBrokerCode::ON_ICC_ACCOUNT_UPDATED) + 1;
    uint32_t code = static_cast<uint32_t>(GetRandomInt(0, maxCode, data, size));
    MessageParcel reply;
    MessageOption option;
    telephonyObserver.OnRemoteRequest(code, dataMessageParcel, reply, option);
}

void CallStateUpdatedInner(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    int32_t maxState = static_cast<int32_t>(Telephony::CallStatus::CALL_STATUS_ANSWERED) + 1;
    int32_t callState = GetRandomInt(-1, maxState, data, size);
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
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
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
    MessageParcel dataMessageParcel;
    dataMessageParcel.WriteInt32(slotId);
    auto networkState = std::make_shared<NetworkState>();
    if (networkState == nullptr) {
        return;
    }
    networkState->isEmergency_ = GetRandomInt(0, 1, data, size);
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
    telephonyObserver.OnNetworkStateUpdatedInner(dataMessageParcel, reply);
}

void CellInfoUpdatedInner(const uint8_t *data, size_t size)
{
    if (!IsServiceInited()) {
        return;
    }
    int32_t slotId = GetRandomInt(MIN_SLOT_ID, MAX_SLOT_ID, data, size);
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
