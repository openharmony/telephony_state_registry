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

#include "telephony_observer_ffi.h"
#include "telephony_observer_impl.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace Telephony {
extern "C" {

    int32_t FfiTelephonyObserverOnNetworkStateChange(ObserverOptions options, int64_t funcId)
    {
        return TelephonyObserverImpl::OnNetworkStateChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffNetworkStateChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffNetworkStateChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllNetworkStateChange()
    {
        return TelephonyObserverImpl::OffAllNetworkStateChange();
    }

    int32_t FfiTelephonyObserverOnSignalInfoChange(ObserverOptions options, int64_t funcId)
    {
        return TelephonyObserverImpl::OnSignalInfoChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffSignalInfoChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffSignalInfoChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllSignalInfoChange()
    {
        return TelephonyObserverImpl::OffAllSignalInfoChange();
    }

    int32_t FfiTelephonyObserverOnCallStateChange(ObserverOptions options, int64_t funcId)
    {
        return TelephonyObserverImpl::OnCallStateChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffCallStateChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffCallStateChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllCallStateChange()
    {
        return TelephonyObserverImpl::OffAllCallStateChange();
    }

    int32_t FfiTelephonyObserverOnCellularDataConnectionStateChange(
        ObserverOptions options,
        int64_t funcId)
    {
        return TelephonyObserverImpl::OnCellularDataConnectionStateChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffCellularDataConnectionStateChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffCellularDataConnectionStateChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllCellularDataConnectionStateChange()
    {
        return TelephonyObserverImpl::OffAllCellularDataConnectionStateChange();
    }

    int32_t FfiTelephonyObserverOnCellularDataFlowChange(ObserverOptions options, int64_t funcId)
    {
        return TelephonyObserverImpl::OnCellularDataFlowChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffCellularDataFlowChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffCellularDataFlowChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllCellularDataFlowChange()
    {
        return TelephonyObserverImpl::OffAllCellularDataFlowChange();
    }

    int32_t FfiTelephonyObserverOnSimStateChange(ObserverOptions options, int64_t funcId)
    {
        return TelephonyObserverImpl::OnSimStateChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffSimStateChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffSimStateChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllSimStateChange()
    {
        return TelephonyObserverImpl::OffAllSimStateChange();
    }

    int32_t FfiTelephonyObserverOnIccAccountInfoChange(ObserverOptions options, int64_t funcId)
    {
        return TelephonyObserverImpl::OnIccAccountInfoChange(ObserverOptions options, int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffIccAccountInfoChange(int64_t funcId)
    {
        return TelephonyObserverImpl::OffIccAccountInfoChange(int64_t funcId);
    }

    int32_t FfiTelephonyObserverOffAllIccAccountInfoChange()
    {
        return TelephonyObserverImpl::OffAllIccAccountInfoChange();
    }


}
}  // namespace Telephony
}  // namespace OHOS
