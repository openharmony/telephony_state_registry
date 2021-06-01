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
#include "napi_telephony_observer.h"
#include "kit_state_registry_hilog_wrapper.h"

namespace OHOS {
namespace TelephonyNapi {
constexpr int ZERO = 0;
constexpr int ONE = 1;
constexpr int TWO = 2;

napi_value CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, (char *)msg.data(), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

napi_value CreateUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

NapiTelephonyObserver::NapiTelephonyObserver(int32_t eventType, std::list<EventListener> &listenerList)
{
    eventType_ = eventType;
    listenerList_ = &listenerList;
}

void NapiTelephonyObserver::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber)
{
    for (std::list<EventListener>::iterator listenerIterator = listenerList_->begin();
         listenerIterator != listenerList_->end(); ++listenerIterator) {
        HILOG_DEBUG("Exec OnCallStateUpdated in the for:");
        if (listenerIterator->eventType == TelephonyNapi::LISTEN_CALL_STATE) {
            HILOG_DEBUG("Exec OnCallStateUpdated in the if: isOnce = %{public}d", listenerIterator->isOnce);
            napi_env env = listenerIterator->env;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            HILOG_DEBUG("Exec OnCallStateUpdated napi_open_handle_scope");
            napi_value thisVar = listenerIterator->thisVar;
            napi_ref callbackRef = listenerIterator->callbackRef;
            napi_value callbackFunc = nullptr;
            napi_get_reference_value(env, callbackRef, &callbackFunc);
            HILOG_DEBUG("Exec OnCallStateUpdated after napi_get_reference_value");
            napi_value callbackResult = nullptr;
            std::string phoneNumberStr = ToUtf8(phoneNumber);
            napi_value callbackValues[TWO] = {0};
            int32_t wrappedCallState = WrapCallState(callState);
            if (wrappedCallState >= TelephonyNapi::CALL_STATE_UNKNOWN) {
                callbackValues[ZERO] = CreateUndefined(env);
                napi_create_object(env, &callbackValues[ONE]);
                SetPropertyInt32(env, callbackValues[ONE], "state", wrappedCallState);
                SetPropertyStringUtf8(env, callbackValues[ONE], "number", phoneNumberStr);
            } else {
                callbackValues[ZERO] = CreateErrorMessage(env, "get data error");
                callbackValues[ONE] = CreateUndefined(env);
            }
            HILOG_DEBUG("Exec OnCallStateUpdated before napi_call_function");
            napi_call_function(
                env, thisVar, callbackFunc, TelephonyNapi::TWO_ARGUMENT, callbackValues, &callbackResult);
            HILOG_DEBUG("Exec OnCallStateUpdated after napi_call_function");
            napi_close_handle_scope(env, scope);
            HILOG_DEBUG("Exec OnCallStateUpdated in the if end ");
        }
        HILOG_DEBUG("Exec OnCallStateUpdated in the for end");
    }
    listenerList_->remove_if([](EventListener listener) -> bool {
        return listener.isOnce && listener.eventType == TelephonyNapi::LISTEN_CALL_STATE;
    });
    HILOG_DEBUG("Exec OnCallStateUpdated  End");
}

void NapiTelephonyObserver::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &signalInfoList)
{
    HILOG_DEBUG("Exec OnSignalInfoUpdated  Start");
    for (std::list<EventListener>::iterator listenerIterator = listenerList_->begin();
         listenerIterator != listenerList_->end(); ++listenerIterator) {
        HILOG_DEBUG("Exec OnSignalInfoUpdated  in the for");
        if (listenerIterator->eventType == TelephonyNapi::LISTEN_SIGNAL_STRENGTHS) {
            napi_env env = listenerIterator->env;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            napi_value thisVar = listenerIterator->thisVar;
            napi_ref callbackRef = listenerIterator->callbackRef;
            napi_value callbackFunc = nullptr;
            napi_get_reference_value(env, callbackRef, &callbackFunc);
            int listSize = signalInfoList.size();
            HILOG_DEBUG(
                "Exec OnSignalInfoUpdated  eventType == TelephonyNapi::LISTEN_SIGNAL_STRENGTHS listSize = "
                "%{public}d",
                listSize);
            napi_value callbackValue[TWO] = {0};
            if (listSize > ZERO) {
                callbackValue[ZERO] = CreateUndefined(env);
                napi_create_array(env, &callbackValue[ONE]);
                for (int i = 0; i < listSize; ++i) {
                    sptr<SignalInformation> inforItem = signalInfoList[i];
                    napi_value info = nullptr;
                    napi_create_object(env, &info);
                    SetPropertyInt32(env, info, "signalType", WrapNetworkType(inforItem->GetNetworkType()));
                    SetPropertyInt32(env, info, "signalLevel", inforItem->GetSignalLevel());
                    napi_set_element(env, callbackValue[ONE], i, info);
                }
            } else {
                callbackValue[ZERO] = CreateErrorMessage(env, "get signal info list failed");
                callbackValue[ONE] = CreateUndefined(env);
            }
            napi_value callbackResult = nullptr;
            napi_call_function(
                env, thisVar, callbackFunc, TelephonyNapi::TWO_ARGUMENT, callbackValue, &callbackResult);
            napi_close_handle_scope(env, scope);
        }
    }
    listenerList_->remove_if([](EventListener listener) -> bool {
        return listener.isOnce && listener.eventType == TelephonyNapi::LISTEN_SIGNAL_STRENGTHS;
    });
    HILOG_DEBUG("Exec OnSignalInfoUpdated  End");
}

void NapiTelephonyObserver::OnNetworkStateUpdated(const sptr<NetworkState> &networkState)
{
    HILOG_DEBUG("Exec OnNetworkStateUpdated  Start");
    for (std::list<EventListener>::iterator listenerIterator = listenerList_->begin();
         listenerIterator != listenerList_->end(); ++listenerIterator) {
        if (listenerIterator->eventType == TelephonyNapi::LISTEN_NET_WORK_STATE) {
            napi_env env = listenerIterator->env;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            napi_value thisVar = listenerIterator->thisVar;
            napi_ref callbackRef = listenerIterator->callbackRef;
            napi_value callbackFunc = nullptr;
            napi_get_reference_value(env, callbackRef, &callbackFunc);
            napi_value callbackValues[TWO] = {0};
            if (networkState != nullptr) {
                callbackValues[ZERO] = CreateUndefined(env);
                napi_value object = nullptr;
                napi_create_object(env, &object);
                SetPropertyStringUtf8(env, object, "longOperatorName", networkState->GetLongOperatorName());
                SetPropertyStringUtf8(env, object, "shortOperatorName", networkState->GetShortOperatorName());
                SetPropertyStringUtf8(env, object, "plmnNumeric", networkState->GetPlmnNumeric());
                SetPropertyInt32(env, object, "isRoaming", networkState->IsRoaming() ? ONE : ZERO);
                SetPropertyInt32(env, object, "regStatus", networkState->GetRegStatus());
                SetPropertyInt32(env, object, "isEmergency", networkState->IsEmergency());
                callbackValues[ONE] = object;
            } else {
                callbackValues[ZERO] = CreateErrorMessage(env, "networkState data error");
                callbackValues[ONE] = CreateUndefined(env);
            }
            napi_value callbackResult = nullptr;
            napi_call_function(
                env, thisVar, callbackFunc, TelephonyNapi::TWO_ARGUMENT, callbackValues, &callbackResult);
            napi_close_handle_scope(env, scope);
        }
    }
    listenerList_->remove_if([](EventListener listener) -> bool {
        return listener.isOnce && listener.eventType == TelephonyNapi::LISTEN_NET_WORK_STATE;
    });
}

std::string NapiTelephonyObserver::ToUtf8(std::u16string str16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(str16);
}

int32_t NapiTelephonyObserver::WrapRadioTech(RadioTech radioTechType)
{
    switch (radioTechType) {
        case OHOS::RADIO_TECHNOLOGY_GSM: {
            return TelephonyNapi::RADIO_TECHNOLOGY_GSM;
        }
        case OHOS::RADIO_TECHNOLOGY_1XRTT: {
            return TelephonyNapi::RADIO_TECHNOLOGY_1XRTT;
        }
        default: {
            return TelephonyNapi::RADIO_TECHNOLOGY_UNKNOWN;
        }
    }
}

int32_t NapiTelephonyObserver::WrapNetworkType(SignalInformation::NetworkType networkType)
{
    int type = static_cast<int>(networkType);
    switch (type) {
        case GSM: {
            return TelephonyNapi::NETWORK_TYPE_GSM;
        }
        case CDMA: {
            return TelephonyNapi::NETWORK_TYPE_CDMA;
        }
        default: {
            return TelephonyNapi::NETWORK_TYPE_UNKNOWN;
        }
    }
}

int32_t NapiTelephonyObserver::WrapCallState(int32_t callState)
{
    switch (callState) {
        case CALL_STATUS_ACTIVE:
        case CALL_STATUS_HOLDING:
        case CALL_STATUS_DIALING:
        case CALL_STATUS_ALERTING:
        case CALL_STATUS_DISCONNECTING:
            return TelephonyNapi::CALL_STATE_OFFHOOK;
        case CALL_STATUS_WAITING:
        case CALL_STATUS_INCOMING:
            return TelephonyNapi::CALL_STATE_RINGING;
        case CALL_STATUS_DISCONNECTED:
        case CALL_STATUS_IDLE:
            return TelephonyNapi::CALL_STATE_IDLE;
        default:
            return TelephonyNapi::CALL_STATE_UNKNOWN;
    }
}

bool NapiTelephonyObserver::MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

bool NapiTelephonyObserver::MatchParamters(
    napi_env env, const napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes)
{
    int i = 0;
    for (auto beg = valueTypes.begin(); beg != valueTypes.end(); ++beg) {
        if (!MatchValueType(env, parameters[i], *beg)) {
            return false;
        }
        ++i;
    }
    return true;
}

bool NapiTelephonyObserver::MatchEventType(const std::string &type, const std::string &goalTypeStr)
{
    return goalTypeStr.compare(type) == 0;
}

int32_t NapiTelephonyObserver::GetEventType(const std::string &type)
{
    if (MatchEventType(type, NET_WORK_STATE_CHANGE)) {
        return LISTEN_NET_WORK_STATE;
    }
    if (MatchEventType(type, CALL_STATE_CHANGE)) {
        return LISTEN_CALL_STATE;
    }
    if (MatchEventType(type, SIGNAL_STRENGTHS_CHANGE)) {
        return LISTEN_SIGNAL_STRENGTHS;
    }
    return NONE_EVENT_TYPE;
}

bool NapiTelephonyObserver::HasEventMask(uint32_t eventType, uint32_t masks)
{
    return (eventType & masks) > 0;
}

void NapiTelephonyObserver::SetPropertyInt32(napi_env env, napi_value object, std::string name, int32_t value)
{
    napi_value peopertyValue = nullptr;
    napi_create_int32(env, value, &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}

void NapiTelephonyObserver::SetPropertyStringUtf8(
    napi_env env, napi_value object, std::string name, std::string value)
{
    napi_value peopertyValue = nullptr;
    char *valueChars = (char *)value.c_str();
    napi_create_string_utf8(env, valueChars, std::strlen(valueChars), &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}
} // namespace TelephonyNapi
} // namespace OHOS