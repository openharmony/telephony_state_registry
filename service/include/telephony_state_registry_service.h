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

#ifndef TELEPHONY_STATE_REGISTRY_SERVICE_H
#define TELEPHONY_STATE_REGISTRY_SERVICE_H

#include <map>
#include <mutex>
#include <string>

#include "singleton.h"
#include "system_ability.h"
#include "common_event_manager.h"
#include "want.h"

#include "telephony_state_registry_record.h"
#include "telephony_state_registry_stub.h"
#include "sim_state_type.h"

namespace OHOS {
namespace Telephony {
enum class ServiceRunningState { STATE_STOPPED, STATE_RUNNING };
class TelephonyStateRegistryService : public SystemAbility,
                                      public TelephonyStateRegistryStub,
                                      public std::enable_shared_from_this<TelephonyStateRegistryService> {
    DECLARE_DELAYED_SINGLETON(TelephonyStateRegistryService)
    DECLARE_SYSTEM_ABILITY(TelephonyStateRegistryService)
public:
    void OnStart() override;
    void OnStop() override;
    void OnDump() override;
    int Dump(std::int32_t fd, const std::vector<std::u16string> &args) override;
    std::string GetBindStartTime();
    std::string GetBindEndTime();
    std::string GetBindSpendTime();
    int32_t UpdateCellularDataConnectState(int32_t slotId, int32_t dataState, int32_t networkType) override;
    int32_t UpdateCellularDataFlow(int32_t slotId, int32_t dataFlowType) override;
    int32_t UpdateCallState(int32_t slotId, int32_t callState, const std::u16string &phoneNumber) override;
    int32_t UpdateCallStateForSlotId(
        int32_t slotId, int32_t callId, int32_t callState, const std::u16string &incomingNumber) override;
    int32_t UpdateSignalInfo(int32_t slotId, const std::vector<sptr<SignalInformation>> &vec) override;
    int32_t UpdateNetworkState(int32_t slotId, const sptr<NetworkState> &networkState) override;
    int32_t UpdateSimState(int32_t slotId, CardType type, SimState state, LockReason reason) override;
    int32_t UpdateCellInfo(int32_t slotId, const std::vector<sptr<CellInformation>> &vec) override;
    int32_t RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver, int32_t slotId,
        uint32_t mask, const std::string &bundleName, bool notifyNow, pid_t pid) override;
    int32_t UnregisterStateChange(int32_t slotId, uint32_t mask, pid_t pid) override;
    int32_t GetServiceRunningState();
    int32_t GetSimState(int32_t slotId);
    int32_t GetCallState(int32_t slotId);
    int32_t GetCardType(int32_t slotId);
    int32_t GetCellularDataConnectionState(int32_t slotId);
    int32_t GetCellularDataFlow(int32_t slotId);
    int32_t GetCellularDataConnectionNetworkType(int32_t slotId);
    int32_t GetLockReason(int32_t slotId);

private:
    void Finalize();
    void UpdateData(const TelephonyStateRegistryRecord &record);

private:
    bool CheckPermission(uint32_t mask);
    bool VerifySlotId(int32_t slotId);
    std::u16string GetCallIncomingNumberForSlotId(TelephonyStateRegistryRecord record, int32_t slotId);
    bool PublishCommonEvent(const AAFwk::Want &want, int32_t eventCode, const std::string &eventData);
    void SendCallStateChanged(int32_t slotId, int32_t state, const std::u16string &number);
    void SendSignalInfoChanged(int32_t slotId, const std::vector<sptr<SignalInformation>> &vec);
    void SendNetworkStateChanged(int32_t slotId, const sptr<NetworkState> &networkState);
    void SendSimStateChanged(int32_t slotId, CardType type, SimState state, LockReason reason);
    void SendCellularDataConnectStateChanged(int32_t slotId, int32_t dataState, int32_t networkType);
    bool IsCommonEventServiceAbilityExist();

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    std::mutex lock_;
    int32_t slotSize_ = 0;
    int64_t bindStartTime_ = 0L;
    int64_t bindEndTime_ = 0L;
    int64_t bindSpendTime_ = 0L;
    std::map<int32_t, int32_t> callState_;
    std::map<int32_t, std::u16string> callIncomingNumber_;
    std::map<int32_t, std::vector<sptr<SignalInformation>>> signalInfos_;
    std::map<int32_t, std::vector<sptr<CellInformation>>> cellInfos_;
    std::map<int32_t, sptr<NetworkState>> searchNetworkState_;
    std::vector<TelephonyStateRegistryRecord> stateRecords_;
    std::map<int32_t, SimState> simState_;
    std::map<int32_t, CardType> cardType_;
    std::map<int32_t, LockReason> simReason_;
    std::map<int32_t, int32_t> cellularDataConnectionState_;
    std::map<int32_t, int32_t> cellularDataConnectionNetworkType_;
    std::map<int32_t, int32_t> cellularDataFlow_;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_STATE_REGISTRY_SERVICE_H
