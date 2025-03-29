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

#ifndef TELEPHONY_OBSERVER_UTILS_H
#define TELEPHONY_OBSERVER_UTILS_H

#include <cstdint>
#include <codecvt>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#include "cell_information.h"
#include "network_state.h"
#include "refbase.h"
#include "signal_information.h"
#include "sim_state_type.h"
#include "telephony_log_wrapper.h"
#include "telephony_observer_broker.h"

namespace OHOS {
namespace Telephony {

    char* MallocCString(const std::string& origin);
    std::string ToUtf8(std::u16string str16);

    enum CJErrorCode {
        /**
         * The input parameter value is out of range.
         */
        CJ_ERROR_TELEPHONY_ARGUMENT_ERROR = 8300001,

        /**
         * Operation failed. Cannot connect to service.
         */
        CJ_ERROR_TELEPHONY_SERVICE_ERROR = 8300002,

        /**
         * System internal error.
         */
        CJ_ERROR_TELEPHONY_SYSTEM_ERROR = 8300003,

        /**
         * Do not have sim card.
         */
        CJ_ERROR_TELEPHONY_NO_SIM_CARD = 8300004,

        /**
         * Airplane mode is on.
         */
        CJ_ERROR_TELEPHONY_AIRPLANE_MODE_ON = 8300005,

        /**
         * Network not in service.
         */
        CJ_ERROR_TELEPHONY_NETWORK_NOT_IN_SERVICE = 8300006,

        /**
         * Unknown error code.
         */
        CJ_ERROR_TELEPHONY_UNKNOW_ERROR = 8300999,

        /**
         * SIM card is not activated.
         */
        CJ_ERROR_SIM_CARD_IS_NOT_ACTIVE = 8301001,

        /**
         * SIM card operation error.
         */
        CJ_ERROR_SIM_CARD_OPERATION_ERROR = 8301002,

        /**
         * Operator config error.
         */
        CJ_ERROR_OPERATOR_CONFIG_ERROR = 8301003,

        /**
         * Permission verification failed, usually the result returned by VerifyAccessToken.
         */
        CJ_ERROR_TELEPHONY_PERMISSION_DENIED = 201,

        /**
         * Permission verification failed, application which is not a system application uses system API.
         */
        CJ_ERROR_ILLEGAL_USE_OF_SYSTEM_API = 202,
    };

    enum class CallState : int32_t {
        /**
         * Indicates an invalid state, which is used when the call state fails to be
         * obtained.
         */
        CALL_STATE_UNKNOWN = -1,

        /**
         * Indicates that there is no ongoing call.
         */
        CALL_STATE_IDLE = 0,

        /**
         * Indicates that an incoming call is ringing or waiting.
         */
        CALL_STATE_RINGING = 1,

        /**
         * Indicates that a least one call is in the dialing, active, or hold state,
         * and there is no new incoming call ringing or waiting.
         */
        CALL_STATE_OFFHOOK = 2,

        /**
         * Indicates that an incoming call is answered.
         */
        CALL_STATE_ANSWERED = 3
    };

    enum class TelephonyUpdateEventType {
        NONE_EVENT_TYPE = 0,
        EVENT_NETWORK_STATE_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE,
        EVENT_CALL_STATE_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE,
        EVENT_CELL_INFO_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO,
        EVENT_SIGNAL_STRENGTHS_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS,
        EVENT_SIM_STATE_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE,
        EVENT_DATA_CONNECTION_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE,
        EVENT_CELLULAR_DATA_FLOW_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_DATA_FLOW,
        EVENT_CFU_INDICATOR_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_CFU_INDICATOR,
        EVENT_VOICE_MAIL_MSG_INDICATOR_UPDATE = TelephonyObserverBroker::OBSERVER_MASK_VOICE_MAIL_MSG_INDICATOR,
        EVENT_ICC_ACCOUNT_CHANGE = TelephonyObserverBroker::OBSERVER_MASK_ICC_ACCOUNT,
    };

    enum class TelephonyCallbackEventId : uint32_t {
        EVENT_REMOVE_ONCE = 0,
        EVENT_ON_CALL_STATE_UPDATE = 1,
        EVENT_ON_SIGNAL_INFO_UPDATE = 2,
        EVENT_ON_NETWORK_STATE_UPDATE = 3,
        EVENT_ON_SIM_STATE_UPDATE = 4,
        EVENT_ON_CELL_INFOMATION_UPDATE = 5,
        EVENT_ON_CELLULAR_DATA_CONNECTION_UPDATE = 6,
        EVENT_ON_CELLULAR_DATA_FLOW_UPDATE = 7,
        EVENT_ON_CFU_INDICATOR_UPDATE = 8,
        EVENT_ON_VOICE_MAIL_MSG_INDICATOR_UPDATE = 9,
        EVENT_ON_ICC_ACCOUNT_UPDATE = 10,
    };

    struct EventListener {
        TelephonyUpdateEventType eventType = TelephonyUpdateEventType::NONE_EVENT_TYPE;
        int32_t slotId = 0;
        int64_t funcId;
        std::function<void(void*)> callbackRef;
        std::shared_ptr<bool> isDeleting = nullptr;
    };

    struct ObserverOptions {
        int32_t slotId = 0;
    };

    struct UpdateInfo {
        int32_t slotId = 0;
        explicit UpdateInfo(int32_t slotId) : slotId(slotId) {}
    };

    struct CallStateUpdateInfo : public UpdateInfo {
        int32_t callState = 0;
        std::u16string phoneNumber_ = u"";
        CallStateUpdateInfo(int32_t slotId, int32_t callStateParam, std::u16string phoneNumberParam)
            : UpdateInfo(slotId), callState(callStateParam), phoneNumber_(phoneNumberParam) {}
    };

    struct SignalUpdateInfo : public UpdateInfo {
        std::vector<sptr<SignalInformation>> signalInfoList_ {};
        SignalUpdateInfo(int32_t slotId, std::vector<sptr<SignalInformation>> infoList)
            : UpdateInfo(slotId), signalInfoList_(infoList) {}
    };

    struct NetworkStateUpdateInfo : public UpdateInfo {
        sptr<NetworkState> networkState_ = nullptr;
        NetworkStateUpdateInfo(int32_t slotId, sptr<NetworkState> state) : UpdateInfo(slotId), networkState_(state) {}
    };

    struct SimStateUpdateInfo : public UpdateInfo {
        CardType type_;
        SimState state_;
        LockReason reason_;
        SimStateUpdateInfo(int32_t slotId, CardType type, SimState simState, LockReason theReason)
            : UpdateInfo(slotId), type_(type), state_(simState), reason_(theReason) {}
    };

    struct CellInfomationUpdate : public UpdateInfo {
        std::vector<sptr<CellInformation>> cellInfoVec_ {};
        CellInfomationUpdate(int32_t slotId, const std::vector<sptr<CellInformation>> &cellInfo)
            : UpdateInfo(slotId), cellInfoVec_(cellInfo) {}
    };

    struct CellularDataConnectState : public UpdateInfo {
        int32_t dataState = 0;
        int32_t networkType = 0;
        CellularDataConnectState(int32_t slotId, int32_t dataState, int32_t networkType)
            : UpdateInfo(slotId), dataState(dataState), networkType(networkType) {}
    };

    struct CellularDataFlowUpdate : public UpdateInfo {
        int32_t flowType = 0;
        CellularDataFlowUpdate(int32_t slotId, int32_t flowType) : UpdateInfo(slotId), flowType(flowType) {}
    };

    struct CfuIndicatorUpdate : public UpdateInfo {
        bool cfuResult_ = false;
        CfuIndicatorUpdate(int32_t slotId, bool cfuResult) : UpdateInfo(slotId), cfuResult_(cfuResult) {}
    };

    struct VoiceMailMsgIndicatorUpdate : public UpdateInfo {
        bool voiceMailMsgResult_ = false;
        VoiceMailMsgIndicatorUpdate(int32_t slotId, bool voiceMailMsgResult)
            : UpdateInfo(slotId), voiceMailMsgResult_(voiceMailMsgResult) {}
    };

    // @C
    struct CNetworkState {
        char* longOperatorName;
        char* shortOperatorName;
        char* plmnNumeric;
        bool isRoaming;
        int32_t regState;
        int32_t cfgTech;
        int32_t nsaState;
        bool isCaActive;
        bool isEmergency;
    };

    struct CSignalInformation {
        int32_t signalType;
        int32_t signalLevel;
        int32_t dBm;
    };

    struct CCallStateInfo {
        int32_t state;
        char* number;
    };

    struct CDataConnectionStateInfo {
        int32_t state;
        int32_t network;
    };

    struct CSimStateData {
        int32_t cardType;
        int32_t state;
        int32_t reason;
    };

    struct CArraySignalInformation {
        CSignalInformation* head;
        int64_t size;
    };
}
}

#endif