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

#ifndef TELEPHONY_OBSERVER_CLIENT_H
#define TELEPHONY_OBSERVER_CLIENT_H

#include <cstdint>
#include <iremote_object.h>
#include <singleton.h>

#include "i_telephony_state_notify.h"

namespace OHOS {
namespace Telephony {
class TelephonyObserverClient : public DelayedRefSingleton<TelephonyObserverClient> {
    DECLARE_DELAYED_REF_SINGLETON(TelephonyObserverClient);

public:
    /**
     * @brief Add state observer.
     *
     * @param telephonyObserver Indicates the TelephonyObserverBroker.
     * @param slotId Indicates the slot identification.
     * @param mask Indicates the event type mask.
     * @param isUpdate Whether to update data immediately.
     * @return Return 0 if add succeed, others if remove failed.
     */
    int32_t AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver,
        int32_t slotId, uint32_t mask, bool isUpdate);

    /**
     * @brief Remove state observer.
     *
     * @param slotId Indicates the slot identification.
     * @param mask Indicates event type mask.
     * @return Return 0 if remove successful, others if remove failed.
     */
    int32_t RemoveStateObserver(int32_t slotId, uint32_t mask);

    /**
     * @brief Get the state registry proxy.
     *
     * @return Return the ITelephonyStateNotify interface.
     */
    sptr<ITelephonyStateNotify> GetProxy();

private:
    class StateRegistryDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit StateRegistryDeathRecipient(TelephonyObserverClient &client) : client_(client) {}
        ~StateRegistryDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        TelephonyObserverClient &client_;
    };

    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutexProxy_;
    sptr<ITelephonyStateNotify> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_OBSERVER_CLIENT_H
