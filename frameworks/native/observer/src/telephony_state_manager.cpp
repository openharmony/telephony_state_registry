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

#include "telephony_state_manager.h"

#include "new"
#include "refbase.h"
#include "singleton.h"
#include "telephony_observer_broker.h"
#include "telephony_observer_client.h"

namespace OHOS {
namespace Telephony {
int32_t TelephonyStateManager::AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t slotId, uint32_t mask, bool notifyNow)
{
    return DelayedRefSingleton<TelephonyObserverClient>::GetInstance().AddStateObserver(
        telephonyObserver, slotId, mask, notifyNow);
}

int32_t TelephonyStateManager::RemoveStateObserver(int32_t slotId, uint32_t mask)
{
    return DelayedRefSingleton<TelephonyObserverClient>::GetInstance().
        RemoveStateObserver(slotId, mask);
}
} // namespace Telephony
} // namespace OHOS