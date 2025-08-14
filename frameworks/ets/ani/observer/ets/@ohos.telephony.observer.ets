/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

/**
 * @file
 * @kit TelephonyKit
 */

import type { Callback } from '@ohos.base';
import type radio from '@ohos.telephony.radio';
import type data from '@ohos.telephony.data';
import type sim from '@ohos.telephony.sim';
import type call from '@ohos.telephony.call';

export default namespace observer {
  loadLibrary("telephony_observer_ani")

  type NetworkState = radio.NetworkState;
  type SignalInformation = radio.SignalInformation;
  type CellInformation = radio.CellInformation;
  type DataConnectState = data.DataConnectState;
  type RatType = radio.RadioTechnology;
  type DataFlowType = data.DataFlowType;
  type CardType = sim.CardType;
  type SimState = sim.SimState;
  type CallState = call.CallState;

  export native function onCellularDataFlowChange(callback: Callback<DataFlowType>): void;
  export native function onCellularDataFlowChangeOptions(options: ObserverOptions, callback: Callback<DataFlowType>): void;
  export native function offCellularDataFlowChange(callback?: Callback<DataFlowType>): void;
  export native function onIccAccountInfoChange(callback: Callback<void>): void;
  export native function offIccAccountInfoChange(callback?: Callback<void>): void;
  export native function onSimStateChange(callback: Callback<SimStateData>): void;
  export native function onSimStateChangeOptions(options: ObserverOptions, callback: Callback<SimStateData>): void;
  export native function offSimStateChange(callback?: Callback<SimStateData>): void;
  export native function onSignalInfoChange(callback: Callback<Array<SignalInformation>>): void;
  export native function onSignalInfoChangeOptions(options: ObserverOptions, callback: Callback<Array<SignalInformation>>): void;
  export native function offSignalInfoChange(callback?: Callback<Array<SignalInformation>>): void;
  export native function onCellInfoChange(callback: Callback<Array<CellInformation>>): void;
  export native function onCellInfoChangeOptions(options: ObserverOptions, callback: Callback<Array<CellInformation>>): void;
  export native function offCellInfoChange(callback?: Callback<Array<CellInformation>>): void;
  export native function onCellularDataConnectionStateChange(callback: Callback<DataConnectionStateInfo>): void;
  export native function onCellularDataConnectionStateChangeOptions(options: ObserverOptions, callback: Callback<DataConnectionStateInfo>): void;
  export native function offCellularDataConnectionStateChange(callback?: Callback<DataConnectionStateInfo>): void;

  export native function onNetworkStateChange(callback: Callback<NetworkState>): void;
  export native function onNetworkStateChangeOptions(options: ObserverOptions, callback: Callback<NetworkState>): void;
  export native function offNetworkStateChange(callback?: Callback<NetworkState>): void;

  export native function onCallStateChange(callback: Callback<CallStateInfo>): void;
  export native function onCallStateChangeOptions(options: ObserverOptions, callback: Callback<CallStateInfo>): void;
  export native function offCallStateChange(callback?: Callback<CallStateInfo>): void;


  export function on(type: string, callback: Object): void {
    if (type == 'cellularDataFlowChange') {
      onCellularDataFlowChange(callback as Callback<DataFlowType>)
    } else if (type == 'iccAccountInfoChange') {
      onIccAccountInfoChange(callback as Callback<void>)
    } else if (type == 'simStateChange') {
      onSimStateChange(callback as Callback<SimStateData>)
    } else if (type == 'signalInfoChange') {
      onSignalInfoChange(callback as Callback<Array<SignalInformation>>)
    } else if (type == 'cellInfoChange') {
      onCellInfoChange(callback as Callback<Array<CellInformation>>)
    } else if (type == 'cellularDataConnectionStateChange') {
      onCellularDataConnectionStateChange(callback as Callback<DataConnectionStateInfo>)
    } else if (type == 'networkStateChange') {
      onNetworkStateChange(callback as Callback<NetworkState>)
    } else if (type == 'callStateChange') {
      onCallStateChange(callback as Callback<CallStateInfo>)
    }
  }

  export function on(type: string, options: ObserverOptions, callback: Object): void {
    if (type == 'cellularDataFlowChange') {
      onCellularDataFlowChangeOptions(options, callback as Callback<DataFlowType>)
    } else if (type == 'simStateChange') {
      onSimStateChangeOptions(options, callback as Callback<SimStateData>)
    } else if (type == 'signalInfoChange') {
      onSignalInfoChangeOptions(options, callback as Callback<Array<SignalInformation>>)
    } else if (type == 'cellInfoChange') {
      onCellInfoChangeOptions(options, callback as Callback<Array<CellInformation>>)
    } else if (type == 'cellularDataConnectionStateChange') {
      onCellularDataConnectionStateChangeOptions(options, callback as Callback<DataConnectionStateInfo>)
    } else if (type == 'networkStateChange') {
      onNetworkStateChangeOptions(options, callback as Callback<NetworkState>)
    } else if (type == 'callStateChange') {
      onCallStateChangeOptions(options, callback as Callback<CallStateInfo>)
    } 
  }

  export function off(type: string, callback?: Object): void {
    if (type == 'cellularDataFlowChange') {
      if (callback == undefined) {
        offCellularDataFlowChange()
      } else {
        offCellularDataFlowChange(callback as Callback<DataFlowType>)
      }
    } else if (type == 'iccAccountInfoChange') {
      if (callback == undefined) {
        offIccAccountInfoChange()
      } else {
        offIccAccountInfoChange(callback as Callback<void>)
      }
    } else if (type == 'simStateChange') {
      if (callback == undefined) {
        offSimStateChange()
      } else {
        offSimStateChange(callback as Callback<SimStateData>)
      }
    } else if (type == 'signalInfoChange') {
      if (callback == undefined) {
        offSignalInfoChange()
      } else {
        offSignalInfoChange(callback as Callback<Array<SignalInformation>>)
      }
    } else if (type == 'cellInfoChange') {
      if (callback == undefined) {
        offCellInfoChange()
      } else {
        offCellInfoChange(callback as Callback<Array<CellInformation>>)
      }
    } else if (type == 'cellularDataConnectionStateChange') {
      if (callback == undefined) {
        offCellularDataConnectionStateChange()
      } else {
        offCellularDataConnectionStateChange(callback as Callback<DataConnectionStateInfo>)
      }
    } else if (type == 'networkStateChange') {
      if (callback == undefined) {
        offNetworkStateChange()
      } else {
        offNetworkStateChange(callback as Callback<NetworkState>)
      }
    } else if (type == 'callStateChange') {
      if (callback == undefined) {
        offCallStateChange()
      } else {
        offCallStateChange(callback as Callback<CallStateInfo>)
      }
    }
  }

  export interface SimStateData {
    type: CardType;
    state: SimState;
    reason: LockReason;
  }

  export class SimStateDataInner implements SimStateData {
    type: CardType;
    state: SimState;
    reason: LockReason;
  }

  export interface DataConnectionStateInfo {
    state: DataConnectState;
    network: RatType;
  }

  export class DataConnectionStateInfoInner implements DataConnectionStateInfo {
    state: DataConnectState;
    network: RatType;
  }

  export interface ObserverOptions {
    slotId: int;
  }

  export class ObserverOptionsInner implements ObserverOptions {
    slotId: int;
  }

  export enum LockReason {
    SIM_NONE,
    SIM_PIN,
    SIM_PUK,
    SIM_PN_PIN,
    SIM_PN_PUK,
    SIM_PU_PIN,
    SIM_PU_PUK,
    SIM_PP_PIN,
    SIM_PP_PUK,
    SIM_PC_PIN,
    SIM_PC_PUK,
    SIM_SIM_PIN,
    SIM_SIM_PUK,
  }

  export interface CallStateInfo {
    state: CallState;
    teleNumber: string;
  }

  export class CallStateInfoInner implements CallStateInfo {
    state: CallState;
    teleNumber: string;
  }

}

