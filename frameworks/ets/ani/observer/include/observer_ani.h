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

#ifndef TELEPHONY_OBSERVER_ANI_H
#define TELEPHONY_OBSERVER_ANI_H

#include <memory>
#include <string>
#include <vector>

#include "signal_information.h"
#include "telephony_observer.h"
#include "telephony_observer_broker.h"

namespace OHOS {
namespace ObserverAni {
struct ArktsError;

enum class TelephonyUpdateEventType {
    NONE_EVENT_TYPE = 0,
    EVENT_NETWORK_STATE_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE,
    EVENT_CALL_STATE_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE,
    EVENT_CELL_INFO_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO,
    EVENT_SIGNAL_STRENGTHS_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS,
    EVENT_SIM_STATE_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE,
    EVENT_DATA_CONNECTION_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE,
    EVENT_CELLULAR_DATA_FLOW_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW,
    EVENT_CFU_INDICATOR_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR,
    EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR,
    EVENT_ICC_ACCOUNT_CHANGE = Telephony::TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT,
};

class AniTelephonyObserver : public Telephony::TelephonyObserver {
public:
    void OnSignalInfoUpdated(int32_t slotId, const std::vector<sptr<Telephony::SignalInformation>> &vec) override;
    void OnNetworkStateUpdated(int32_t slotId, const sptr<Telephony::NetworkState> &networkState) override;
    void OnSimStateUpdated(int32_t slotId, Telephony::CardType type, Telephony::SimState state,
                           Telephony::LockReason reason) override;
    void OnCellInfoUpdated(int32_t slotId, const std::vector<sptr<Telephony::CellInformation>> &vec) override;
    void OnCellularDataConnectStateUpdated(int32_t slotId, int32_t dataState, int32_t networkType) override;
    void OnCellularDataFlowUpdated(int32_t slotId, int32_t dataFlowType) override;
    void OnIccAccountUpdated() override;
    void OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;
};

bool IsValidSlotIdEx(int32_t slotId, uint32_t eventType);
ArktsError EventListenerRegister(int32_t slotId, uint32_t eventType);
ArktsError EventListenerUnRegister(int32_t slotId, uint32_t eventType);

} //namespace ObserverAni
} //namespace OHOS

#endif
