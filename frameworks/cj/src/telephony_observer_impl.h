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

#ifndef TELEPHONY_OBSERVER_IMPL_H
#define TELEPHONY_OBSERVER_IMPL_H

#include "observer_event_handler.h"
#include "telephony_observer.h"
#include "telephony_observer_broker.h"
#include "telephony_observer_utils.h"
#include "telephony_types.h"

namespace OHOS {
namespace Telephony {

class TelephonyObserverImpl {
public:
    template<typename T, typename D>
    inline static bool SendEvent(uint32_t innerEventId, std::unique_ptr<T, D> &object, int64_t delayTime = 0)
    {
        auto handler = DelayedSingleton<ObserverEventHandler>::GetInstance();
        if (handler == nullptr) {
            TELEPHONY_LOGE("Get handler failed");
            return false;
        }
        return handler->SendEvent(innerEventId, object, delayTime);
    }

    inline static bool SendEvent(uint32_t innerEventId)
    {
        auto handler = DelayedSingleton<ObserverEventHandler>::GetInstance();
        if (handler == nullptr) {
            TELEPHONY_LOGE("Get handler failed");
            return false;
        }
        return handler->SendEvent(innerEventId);
    }

    // static int32_t RegisterEventListener(EventListener &eventListener);
    // static int32_t UnregisterEventListener(const TelephonyUpdateEventType eventType, int64_t funcId);

    static int32_t OnNetworkStateChange(ObserverOptions options, int64_t funcId);
    static int32_t OffNetworkStateChange(int64_t funcId);
    static int32_t OffAllNetworkStateChange();

    static int32_t OnSignalInfoChange(ObserverOptions options, int64_t funcId);
    static int32_t OffSignalInfoChange(int64_t funcId);
    static int32_t OffAllSignalInfoChange();

    static int32_t OnCallStateChange(ObserverOptions options, int64_t funcId);
    static int32_t OffCallStateChange(int64_t funcId);
    static int32_t OffAllCallStateChange();

    static int32_t OnCellularDataConnectionStateChange(ObserverOptions options, int64_t funcId);
    static int32_t OffCellularDataConnectionStateChange(int64_t funcId);
    static int32_t OffAllCellularDataConnectionStateChange();

    static int32_t OnCellularDataFlowChange(ObserverOptions options, int64_t funcId);
    static int32_t OffCellularDataFlowChange(int64_t funcId);
    static int32_t OffAllCellularDataFlowChange();

    static int32_t OnSimStateChange(ObserverOptions options, int64_t funcId);
    static int32_t OffSimStateChange(int64_t funcId);
    static int32_t OffAllSimStateChange();

    static int32_t OnIccAccountInfoChange(ObserverOptions options, int64_t funcId);
    static int32_t OffIccAccountInfoChange(int64_t funcId);
    static int32_t OffAllIccAccountInfoChange();

};

class FfiTelephonyObserver: public TelephonyObserver {
public:
    void OnCallStateUpdated(int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;
    void OnSignalInfoUpdated(int32_t slotId, const std::vector<sptr<SignalInformation>> &vec) override;
    void OnNetworkStateUpdated(int32_t slotId, const sptr<NetworkState> &networkState) override;
    void OnSimStateUpdated(int32_t slotId, CardType type, SimState state, LockReason reason) override;
    void OnCellInfoUpdated(int32_t slotId, const std::vector<sptr<CellInformation>> &vec) override;
    void OnCellularDataConnectStateUpdated(int32_t slotId, int32_t dataState, int32_t networkType) override;
    void OnCellularDataFlowUpdated(int32_t slotId, int32_t dataFlowType) override;
    void OnCfuIndicatorUpdated(int32_t slotId, bool cfuResult) override;
    void OnVoiceMailMsgIndicatorUpdated(int32_t slotId, bool voiceMailMsgResult) override;
    void OnIccAccountUpdated() override;
};
}
}

#endif