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

use serde::{Deserialize, Serialize};

pub const DEFAULT_SIM_SLOT_ID: i32 = 0;

#[ani_rs::ani(path = "@ohos.telephony.data.data.DataFlowType")]
#[derive(Debug, Clone)]
#[repr(i32)]
pub enum DataFlowType {
    DataFlowTypeNone = 0,
    DataFlowTypeDown = 1,
    DataFlowTypeUp = 2,
    DataFlowTypeUpDown = 3,
    DataFlowTypeDormant = 4,
}

impl From<i32> for DataFlowType {
    fn from(value: i32) -> Self {
        match value {
            0 => DataFlowType::DataFlowTypeNone,
            1 => DataFlowType::DataFlowTypeDown,
            2 => DataFlowType::DataFlowTypeUp,
            3 => DataFlowType::DataFlowTypeUpDown,
            4 => DataFlowType::DataFlowTypeDormant,
            _ => DataFlowType::DataFlowTypeNone,
        }
    }
}

#[ani_rs::ani]
#[derive(Debug)]
pub struct ObserverOptions {
    pub slot_id: i32,
}

#[ani_rs::ani(path = "@ohos.telephony.sim.sim.SimState")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum SimState {
    SimStateUnknown = 0,
    SimStateNotPresent = 1,
    SimStateLocked = 2,
    SimStateNotReady = 3,
    SimStateReady = 4,
    SimStateLoaded = 5,
}

impl From<i32> for SimState {
    fn from(value: i32) -> Self {
        match value {
            0 => SimState::SimStateUnknown,
            1 => SimState::SimStateNotPresent,
            2 => SimState::SimStateLocked,
            3 => SimState::SimStateNotReady,
            4 => SimState::SimStateReady,
            5 => SimState::SimStateLoaded,
            _ => SimState::SimStateUnknown,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.observer.observer.LockReason")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum LockReason {
    SimNone,
    SimPin,
    SimPuk,
    SimPnPin,
    SimPnPuk,
    SimPuPin,
    SimPuPuk,
    SimPpPin,
    SimPpPuk,
    SimPcPin,
    SimPcPuk,
    SimSimPin,
    SimSimPuk,
}

impl From<i32> for LockReason {
    fn from(value: i32) -> Self {
        match value {
            0 => LockReason::SimNone,
            1 => LockReason::SimPin,
            2 => LockReason::SimPuk,
            3 => LockReason::SimPnPin,
            4 => LockReason::SimPnPuk,
            5 => LockReason::SimPuPin,
            6 => LockReason::SimPuPuk,
            7 => LockReason::SimPpPin,
            8 => LockReason::SimPpPuk,
            9 => LockReason::SimPcPin,
            10 => LockReason::SimPcPuk,
            11 => LockReason::SimSimPin,
            12 => LockReason::SimSimPuk,
            _ => LockReason::SimNone,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.sim.sim.CardType")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum CardType {
    UnknownCard = -1,
    SingleModeSimCard = 10,
    SingleModeUsimCard = 20,
    SingleModeRuimCard = 30,
    DualModeCgCard = 40,
    CtNationalRoamingCard = 41,
    CuDualModeCard = 42,
    DualModeTelecomLteCard = 43,
    DualModeUgCard = 50,
    SingleModeIsimCard = 60,
}

impl From<i32> for CardType {
    fn from(value: i32) -> Self {
        match value {
            -1 => CardType::UnknownCard,
            10 => CardType::SingleModeSimCard,
            20 => CardType::SingleModeUsimCard,
            30 => CardType::SingleModeRuimCard,
            40 => CardType::DualModeCgCard,
            41 => CardType::CtNationalRoamingCard,
            42 => CardType::CuDualModeCard,
            43 => CardType::DualModeTelecomLteCard,
            50 => CardType::DualModeUgCard,
            60 => CardType::SingleModeIsimCard,
            _ => CardType::UnknownCard,
        }
    }
}

#[derive(Serialize, Deserialize)]
#[serde(rename = "@ohos.telephony.observer.observer.SimStateDataInner\0")]
#[derive(Debug, Clone)]
pub struct SimStateData {
    #[serde(rename = "type\0")]
    pub card_type: CardType,
    #[serde(rename = "state\0")]
    pub state: SimState,
    #[serde(rename = "reason\0")]
    pub reason: LockReason,
}

impl SimStateData {
    pub fn new(card_type: CardType, state: SimState, reason: LockReason) -> Self {
        Self {
            card_type,
            state,
            reason,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.NetworkType")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum NetworkType {
    NetworkTypeUnknown = 0,
    NetworkTypeGsm = 1,
    NetworkTypeCdma = 2,
    NetworkTypeWcdma = 3,
    NetworkTypeTdscdma = 4,
    NetworkTypeLte = 5,
    NetworkTypeNr = 6,
}

impl From<i32> for NetworkType {
    fn from(value: i32) -> Self {
        match value {
            0 => NetworkType::NetworkTypeUnknown,
            1 => NetworkType::NetworkTypeGsm,
            2 => NetworkType::NetworkTypeCdma,
            3 => NetworkType::NetworkTypeWcdma,
            4 => NetworkType::NetworkTypeTdscdma,
            5 => NetworkType::NetworkTypeLte,
            6 => NetworkType::NetworkTypeNr,
            _ => NetworkType::NetworkTypeUnknown,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.SignalInformationInner")]
#[derive(Debug, Clone)]
pub struct SignalInformation {
    signal_type: NetworkType,
    signal_level: i32,
    d_bm: i32,
}

impl SignalInformation {
    pub fn new(signal_type: i32, signal_level: i32, d_bm: i32) -> Self {
        Self {
            signal_type: NetworkType::from(signal_type),
            signal_level,
            d_bm,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.CellInformationInner")]
#[derive(Debug, Clone)]
pub struct CellInformation {
    network_type: NetworkType,
    signal_information: SignalInformation,
}

impl CellInformation {
    pub fn new(network_type: i32, signal_information: SignalInformation) -> Self {
        Self {
            network_type: NetworkType::from(network_type),
            signal_information,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.data.data.DataConnectState")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum DataConnectState {
    DataStateUnknown = -1,
    DataStateDisconnected = 0,
    DataStateConnecting = 1,
    DataStateConnected = 2,
    DataStateSuspended = 3,
}

impl From<i32> for DataConnectState {
    fn from(value: i32) -> Self {
        match value {
            0 => DataConnectState::DataStateDisconnected,
            1 => DataConnectState::DataStateConnecting,
            2 => DataConnectState::DataStateConnected,
            3 => DataConnectState::DataStateSuspended,
            _ => DataConnectState::DataStateUnknown,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.RadioTechnology")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum RadioTechnology {
    RadioTechnologyUnknown = 0,
    RadioTechnologyGsm = 1,
    RadioTechnology_1xrtt = 2,
    RadioTechnologyWcdma = 3,
    RadioTechnologyHspa = 4,
    RadioTechnologyHspap = 5,
    RadioTechnologyTdScdma = 6,
    RadioTechnologyEvdo = 7,
    RadioTechnologyEhrpd = 8,
    RadioTechnologyLte = 9,
    RadioTechnologyLteCa = 10,
    RadioTechnologyIwlan = 11,
    RadioTechnologyNr = 12,
}

impl From<i32> for RadioTechnology {
    fn from(value: i32) -> Self {
        match value {
            0 => Self::RadioTechnologyUnknown,
            1 => Self::RadioTechnologyGsm,
            2 => Self::RadioTechnology_1xrtt,
            3 => Self::RadioTechnologyWcdma,
            4 => Self::RadioTechnologyHspa,
            5 => Self::RadioTechnologyHspap,
            6 => Self::RadioTechnologyTdScdma,
            7 => Self::RadioTechnologyEvdo,
            8 => Self::RadioTechnologyEhrpd,
            9 => Self::RadioTechnologyLte,
            10 => Self::RadioTechnologyLteCa,
            11 => Self::RadioTechnologyIwlan,
            12 => Self::RadioTechnologyNr,
            _ => Self::RadioTechnologyUnknown,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.observer.observer.DataConnectionStateInfoInner")]
#[derive(Debug, Clone)]
pub struct DataConnectionStateInfo {
    state: DataConnectState,
    network: RadioTechnology,
}

impl DataConnectionStateInfo {
    pub fn new(state: i32, network: i32) -> Self {
        Self {
            state: DataConnectState::from(state),
            network: RadioTechnology::from(network),
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.RegState")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum RegState {
    RegStateNoService = 0,
    RegStateInService = 1,
    RegStateEmergencyCallOnly = 2,
    RegStatePowerOff = 3,
}

impl From<i32> for RegState {
    fn from(value: i32) -> Self {
        match value {
            0 => RegState::RegStateNoService,
            1 => RegState::RegStateInService,
            2 => RegState::RegStateEmergencyCallOnly,
            3 => RegState::RegStatePowerOff,
            _ => RegState::RegStateNoService,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.NsaState")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum NsaState {
    NsaStateNotSupport = 1,
    NsaStateNoDetect = 2,
    NsaStateConnectedDetect = 3,
    NsaStateIdleDetect = 4,
    NsaStateDualConnected = 5,
    NsaStateSaAttached = 6,
}

impl From<i32> for NsaState {
    fn from(value: i32) -> Self {
        match value {
            1 => NsaState::NsaStateNotSupport,
            2 => NsaState::NsaStateNoDetect,
            3 => NsaState::NsaStateConnectedDetect,
            4 => NsaState::NsaStateIdleDetect,
            5 => NsaState::NsaStateDualConnected,
            6 => NsaState::NsaStateSaAttached,
            _ => NsaState::NsaStateNotSupport,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.radio.radio.NetworkStateInner")]
#[derive(Debug, Clone)]
pub struct NetworkState {
    pub long_operator_name: String,
    pub short_operator_name: String,
    pub plmn_numeric: String,
    pub is_roaming: bool,
    pub reg_state: RegState,
    pub cfg_tech: RadioTechnology,
    pub nsa_state: NsaState,
    pub is_emergency: bool,
}

#[ani_rs::ani(path = "@ohos.telephony.call.call.CallState")]
#[repr(i32)]
#[derive(Debug, Clone)]
pub enum CallState {
    CallStateUnknown = -1,
    CallStateIdle = 0,
    CallStateRinging = 1,
    CallStateOffhook = 2,
    CallStateAnswered = 3,
}

impl From<i32> for CallState {
    fn from(value: i32) -> Self {
        match value {
            -1 => CallState::CallStateUnknown,
            0 => CallState::CallStateIdle,
            1 => CallState::CallStateRinging,
            2 => CallState::CallStateOffhook,
            3 => CallState::CallStateAnswered,
            _ => CallState::CallStateUnknown,
        }
    }
}

#[ani_rs::ani(path = "@ohos.telephony.observer.observer.CallStateInfoInner")]
#[derive(Debug, Clone)]
pub struct CallStateInfo {
    pub state: CallState,
    pub teleNumber: String,
}

impl CallStateInfo {
    pub fn new(state: i32, teleNumber: String) -> Self {
        Self {
            state: CallState::from(state),
            teleNumber,
        }
    }
}