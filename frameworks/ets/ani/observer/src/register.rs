// Copyright (C) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

use crate::{bridge, telephony_error, wrapper};
use ani_rs::{business_error::BusinessError, objects::GlobalRefCallback};
use std::{
    collections::BTreeSet,
    ops::Deref,
    sync::{Mutex, OnceLock},
};

#[derive(PartialEq, Eq)]
pub enum CallbackFlavor {
    CellularDataFlowChange(GlobalRefCallback<(bridge::DataFlowType,)>),
    IccAccountInfoChange(GlobalRefCallback<()>),
    SimStateChange(GlobalRefCallback<(bridge::SimStateData,)>),
    SignalInfoChange(GlobalRefCallback<(Vec<bridge::SignalInformation>,)>),
    CellInfoChange(GlobalRefCallback<(Vec<bridge::CellInformation>,)>),
    CellularDataConnectionStateChange(GlobalRefCallback<(bridge::DataConnectionStateInfo,)>),
    NetworkStateChange(GlobalRefCallback<(bridge::NetworkState,)>),
    CallStateChange(GlobalRefCallback<(bridge::CallStateInfo,)>),
}

#[derive(PartialEq, Eq)]
#[repr(i32)]
pub enum TelephonyUpdateEventType {
    NoneEventType = 0,
    EventNetworkStateUpdate = 0x00000001,
    EventCallStateUpdate = 0x00000004,
    EventCellInfoUpdate = 0x00000008,
    EventSignalStrengthsUpdate = 0x00000010,
    EventSimStateUpdate = 0x00000020,
    EventDataConnectionUpdate = 0x00000040,
    EventCellularDataFlowUpdate = 0x00000080,
    EventCfuIndicatorUpdate = 0x00000100,
    EventVoiceMailMsgIndicatorUpdate = 0x00000200,
    EventIccAccountChange = 0x00000400,
}

impl TelephonyUpdateEventType {
    pub fn to_u32(&self) -> u32 {
        match self {
            TelephonyUpdateEventType::NoneEventType => 0,
            TelephonyUpdateEventType::EventNetworkStateUpdate => 0x00000001,
            TelephonyUpdateEventType::EventCallStateUpdate => 0x00000004,
            TelephonyUpdateEventType::EventCellInfoUpdate => 0x00000008,
            TelephonyUpdateEventType::EventSignalStrengthsUpdate => 0x00000010,
            TelephonyUpdateEventType::EventSimStateUpdate => 0x00000020,
            TelephonyUpdateEventType::EventDataConnectionUpdate => 0x00000040,
            TelephonyUpdateEventType::EventCellularDataFlowUpdate => 0x00000080,
            TelephonyUpdateEventType::EventCfuIndicatorUpdate => 0x00000100,
            TelephonyUpdateEventType::EventVoiceMailMsgIndicatorUpdate => 0x00000200,
            TelephonyUpdateEventType::EventIccAccountChange => 0x00000400,
        }
    }
}

pub struct EventListener {
    event_type: TelephonyUpdateEventType,
    slot_id: i32,
    callback_ref: CallbackFlavor,
}

impl EventListener {
    pub fn new(
        event_type: TelephonyUpdateEventType,
        slot_id: i32,
        callback_ref: CallbackFlavor,
    ) -> Self {
        Self {
            event_type,
            slot_id,
            callback_ref,
        }
    }
}

#[derive(PartialEq, Eq)]
enum EventListenerStatus {
    EventListenerDiff,
    EventListenerSame,
    EventListenerSlotidAndEventtypeSame,
}

pub struct Register {
    inner: Mutex<Vec<EventListener>>,
}

impl Register {
    fn new() -> Self {
        Self {
            inner: Mutex::new(Vec::new()),
        }
    }

    pub fn get_instance() -> &'static Self {
        static INSTANCE: OnceLock<Register> = OnceLock::new();
        INSTANCE.get_or_init(Register::new)
    }

    fn check_event_listener_register(
        listener_list: &Vec<EventListener>,
        listener: &EventListener,
    ) -> EventListenerStatus {
        let mut flag = EventListenerStatus::EventListenerDiff;
        for listen_item in listener_list {
            if listen_item.slot_id == listener.slot_id
                && listen_item.event_type == listener.event_type
                && listen_item.callback_ref == listener.callback_ref
            {
                flag = EventListenerStatus::EventListenerSame;
                return flag;
            }

            if listen_item.slot_id == listener.slot_id
                && listen_item.event_type == listener.event_type
            {
                flag = EventListenerStatus::EventListenerSlotidAndEventtypeSame;
            }
        }
        return flag;
    }

    pub fn register(&self, listener: EventListener) -> Result<(), BusinessError> {
        if !wrapper::ffi::IsValidSlotIdEx(listener.slot_id, listener.event_type.to_u32()) {
            telephony_error!("Register slotId {} is invalid", listener.slot_id);
            return Err(BusinessError::new_static(8300001, "Invalid parameter value."));
        }

        let mut inner = self.inner.lock().unwrap();
        let register_status = Self::check_event_listener_register(&inner, &listener);
        if register_status == EventListenerStatus::EventListenerSame {
            telephony_error!("RegisterEventListener Callback is already registered.");
            return Ok(());
        }
        if register_status != EventListenerStatus::EventListenerSlotidAndEventtypeSame {
            let arkts_error =
                wrapper::ffi::EventListenerRegister(listener.slot_id, listener.event_type.to_u32());
            if arkts_error.is_error() {
                telephony_error!(
                    "EventListenerRegister error, {}, {}",
                    arkts_error.errorCode,
                    arkts_error.errorMessage
                );
                return Err(BusinessError::from(arkts_error));
            }
        }

        inner.push(listener);
        Ok(())
    }

    fn check_event_type_exist(
        listener_list: &Vec<EventListener>,
        slot_id: i32,
        event_type: &TelephonyUpdateEventType,
    ) -> bool {
        for listen_item in listener_list {
            if listen_item.slot_id == slot_id && listen_item.event_type == *event_type {
                return true;
            }
        }
        false
    }

    pub fn unregister(
        &self,
        event_type: TelephonyUpdateEventType,
        callback_ref: Option<CallbackFlavor>,
    ) -> Result<(), BusinessError> {
        let mut inner = self.inner.lock().unwrap();
        if inner.is_empty() {
            telephony_error!("UnregisterEventListener listener Vec is empty.");
            return Ok(());
        }

        let mut slot_id_set = BTreeSet::new();
        if let Some(callback) = callback_ref {
            inner.retain(|listener| {
                if listener.event_type == event_type && listener.callback_ref == callback {
                    slot_id_set.insert(listener.slot_id);
                    return false;
                } else {
                    return true;
                }
            });
        } else {
            inner.retain(|listener| {
                if listener.event_type == event_type {
                    slot_id_set.insert(listener.slot_id);
                    return false;
                } else {
                    return true;
                }
            });
        }
        for slot_id in slot_id_set {
            if !Self::check_event_type_exist(&inner, slot_id, &event_type) {
                let arkts_error =
                    wrapper::ffi::EventListenerUnRegister(slot_id, event_type.to_u32());
                if arkts_error.is_error() {
                    telephony_error!(
                        "EventListenerUnRegister error, {}, {}",
                        arkts_error.errorCode,
                        arkts_error.errorMessage
                    );
                    return Err(BusinessError::from(arkts_error));
                }
            }
        }
        Ok(())
    }

    pub fn execute_on_cellular_data_flow_change(&self, slot_id: i32, param: i32) {
        let inner = self.inner.lock().unwrap();
        let argv = bridge::DataFlowType::from(param);
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventCellularDataFlowUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::CellularDataFlowChange(func) = &listen_item.callback_ref {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not CellularDataFlowChange callback");
            }
        }
    }

    pub fn execute_on_icc_account_info_change(&self) {
        let inner = self.inner.lock().unwrap();
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventIccAccountChange {
                continue;
            }

            if let CallbackFlavor::IccAccountInfoChange(func) = &listen_item.callback_ref {
                func.execute(());
            } else {
                telephony_error!("Execute is not IccAccountInfoChange callback");
            }
        }
    }

    pub fn execute_on_sim_state_change(
        &self,
        slot_id: i32,
        card_type: i32,
        state: i32,
        reason: i32,
    ) {
        let inner = self.inner.lock().unwrap();
        let card_type = bridge::CardType::from(card_type);
        let state = bridge::SimState::from(state);
        let reason = bridge::LockReason::from(reason);
        let argv = bridge::SimStateData::new(card_type, state, reason);
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventSimStateUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::SimStateChange(func) = &listen_item.callback_ref {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not SimStateChange callback");
            }
        }
    }

    pub fn execute_on_signal_info_change(
        &self,
        slot_id: i32,
        signal_info_list: Vec<wrapper::ffi::SignalInformationAni>,
    ) {
        let inner = self.inner.lock().unwrap();

        let argv: Vec<bridge::SignalInformation> = signal_info_list
            .into_iter()
            .map(|data| data.into())
            .collect();
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventSignalStrengthsUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::SignalInfoChange(func) = &listen_item.callback_ref {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not SignalInfoChange callback");
            }
        }
    }

    pub fn execute_on_cell_info_change(
        &self,
        slot_id: i32,
        cell_info_list: Vec<wrapper::ffi::CellInformationAni>,
    ) {
        let inner = self.inner.lock().unwrap();

        let argv: Vec<bridge::CellInformation> =
            cell_info_list.into_iter().map(|data| data.into()).collect();
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventCellInfoUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::CellInfoChange(func) = &listen_item.callback_ref {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not CellInfoChange callback");
            }
        }
    }

    pub fn execute_on_cellular_data_connection_state_change(
        &self,
        slot_id: i32,
        data_state: i32,
        network_type: i32,
    ) {
        let inner = self.inner.lock().unwrap();
        let argv = bridge::DataConnectionStateInfo::new(data_state, network_type);
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventDataConnectionUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::CellularDataConnectionStateChange(func) =
                &listen_item.callback_ref
            {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not CellularDataConnectionStateChange callback");
            }
        }
    }

    pub fn execute_on_network_state_change(
        &self,
        slot_id: i32,
        network_state: wrapper::ffi::NetworkStateAni,
    ) {
        let inner = self.inner.lock().unwrap();
        let argv = bridge::NetworkState::from(network_state);
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventNetworkStateUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::NetworkStateChange(func) = &listen_item.callback_ref {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not NetworkStateChange callback");
            }
        }
    }

    pub fn execute_on_call_state_change(
        &self,
        slot_id: i32,
        state: wrapper::ffi::CallStateAni,
    ) {
        let inner = self.inner.lock().unwrap();
        let argv = bridge::CallStateInfo::from(call_state);
        if inner.is_empty() {
            telephony_error!("Callback vec is empty");
            return;
        }
        
        for listen_item in inner.deref() {
            if listen_item.event_type != TelephonyUpdateEventType::EventCallStateUpdate
                || listen_item.slot_id != slot_id
            {
                continue;
            }

            if let CallbackFlavor::CallStateChange(func) = &listen_item.callback_ref {
                func.execute((argv.clone(),));
            } else {
                telephony_error!("Execute is not CallStateChange callback");
            }
        }
    }

}

pub fn on_cellular_data_flow_updated(slot_id: i32, data_flow_type: i32) {
    Register::get_instance().execute_on_cellular_data_flow_change(slot_id, data_flow_type);
}

pub fn on_icc_account_updated() {
    Register::get_instance().execute_on_icc_account_info_change();
}

pub fn on_sim_state_updated(slot_id: i32, card_type: i32, state: i32, reason: i32) {
    Register::get_instance().execute_on_sim_state_change(slot_id, card_type, state, reason);
}

pub fn on_signal_info_updated(
    slot_id: i32,
    signal_info_list: Vec<wrapper::ffi::SignalInformationAni>,
) {
    Register::get_instance().execute_on_signal_info_change(slot_id, signal_info_list);
}

pub fn on_cell_info_updated(slot_id: i32, cell_info_list: Vec<wrapper::ffi::CellInformationAni>) {
    Register::get_instance().execute_on_cell_info_change(slot_id, cell_info_list);
}

pub fn on_cellular_data_connect_state_updated(slot_id: i32, data_state: i32, network_type: i32) {
    Register::get_instance().execute_on_cellular_data_connection_state_change(
        slot_id,
        data_state,
        network_type,
    );
}

pub fn on_network_state_updated(slot_id: i32, network_state: wrapper::ffi::NetworkStateAni) {
    Register::get_instance().execute_on_network_state_change(slot_id, network_state);
}

pub fn on_call_state_updated(slot_id: i32, call_state: wrapper::ffi::CallStateAni) {
    Register::get_instance().execute_on_call_state_change(slot_id, call_state);
}

