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

#ifndef TELEPHONY_OBSERVER_FFI_H
#define TELEPHONY_OBSERVER_FFI_H

#include "ffi_remote_data.h"
#include "telephony_observer_utils.h"

namespace OHOS {
namespace Telephony {
extern "C" {
    FFI_EXPORT int32_t FfiTelephonyObserverOnNetworkStateChange(ObserverOptions options, int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffNetworkStateChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllNetworkStateChange();
    FFI_EXPORT int32_t FfiTelephonyObserverOnSignalInfoChange(ObserverOptions options, int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffSignalInfoChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllSignalInfoChange();
    FFI_EXPORT int32_t FfiTelephonyObserverOnCallStateChange(ObserverOptions options, int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffCallStateChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllCallStateChange();
    FFI_EXPORT int32_t FfiTelephonyObserverOnCellularDataConnectionStateChange(
        ObserverOptions options,
        int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffCellularDataConnectionStateChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllCellularDataConnectionStateChange();
    FFI_EXPORT int32_t FfiTelephonyObserverOnCellularDataFlowChange(ObserverOptions options, int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffCellularDataFlowChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllCellularDataFlowChange();
    FFI_EXPORT int32_t FfiTelephonyObserverOnSimStateChange(ObserverOptions options, int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffSimStateChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllSimStateChange();
    FFI_EXPORT int32_t FfiTelephonyObserverOnIccAccountInfoChange(ObserverOptions options, int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffIccAccountInfoChange(int64_t funcId);
    FFI_EXPORT int32_t FfiTelephonyObserverOffAllIccAccountInfoChange();
}
}
}

#endif