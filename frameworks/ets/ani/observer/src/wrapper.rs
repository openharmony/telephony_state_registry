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

use crate::bridge;
use crate::register::*;
use ani_rs::business_error::BusinessError;
use ffi::ArktsError;

pub const TELEPHONY_SUCCESS: i32 = 8300000;

impl From<ffi::SignalInformationAni> for bridge::SignalInformation {
    fn from(value: ffi::SignalInformationAni) -> Self {
        Self::new(value.signal_type, value.signal_level, value.d_bm)
    }
}

impl From<ffi::CellInformationAni> for bridge::CellInformation {
    fn from(value: ffi::CellInformationAni) -> Self {
        let signal_information = bridge::SignalInformation::from(value.signal_information);
        Self::new(value.network_type, signal_information)
    }
}

impl From<ffi::NetworkStateAni> for bridge::NetworkState {
    fn from(value: ffi::NetworkStateAni) -> Self {
        Self {
            long_operator_name: value.long_operator_name,
            short_operator_name: value.short_operator_name,
            plmn_numeric: value.plmn_numeric,
            is_roaming: value.is_roaming,
            reg_state: bridge::RegState::from(value.reg_state),
            cfg_tech: bridge::RadioTechnology::from(value.cfg_tech),
            nsa_state: bridge::NsaState::from(value.nsa_state),
            is_emergency: value.is_emergency,
        }
    }
}

#[cxx::bridge(namespace = "OHOS::ObserverAni")]
pub(crate) mod ffi {
    struct ArktsError {
        pub errorCode: i32,
        pub errorMessage: String,
    }

    struct SignalInformationAni {
        signal_type: i32,
        signal_level: i32,
        d_bm: i32,
    }

    struct CellInformationAni {
        network_type: i32,
        signal_information: SignalInformationAni,
    }

    struct NetworkStateAni {
        long_operator_name: String,
        short_operator_name: String,
        plmn_numeric: String,
        is_roaming: bool,
        reg_state: i32,
        cfg_tech: i32,
        nsa_state: i32,
        is_emergency: bool,
    }

    extern "Rust" {
        fn on_cellular_data_flow_updated(slot_id: i32, data_flow_type: i32);
        fn on_icc_account_updated();
        fn on_sim_state_updated(slot_id: i32, card_type: i32, state: i32, reason: i32);
        fn on_signal_info_updated(slot_id: i32, signal_info_list: Vec<SignalInformationAni>);
        fn on_cell_info_updated(slot_id: i32, cell_info_list: Vec<CellInformationAni>);
        fn on_cellular_data_connect_state_updated(slot_id: i32, data_state: i32, network_type: i32);
        fn on_network_state_updated(slot_id: i32, network_state: NetworkStateAni);
    }

    unsafe extern "C++" {
        include!("observer_ani.h");
        fn IsValidSlotIdEx(slotId: i32, eventType: u32) -> bool;
        fn EventListenerRegister(slotId: i32, eventType: u32) -> ArktsError;
        fn EventListenerUnRegister(slotId: i32, eventType: u32) -> ArktsError;
    }
}

impl ArktsError {
    pub fn is_error(&self) -> bool {
        if self.errorCode != TELEPHONY_SUCCESS {
            return true;
        }
        false
    }
}

impl From<ArktsError> for BusinessError {
    fn from(value: ArktsError) -> Self {
        BusinessError::new(value.errorCode, value.errorMessage)
    }
}
