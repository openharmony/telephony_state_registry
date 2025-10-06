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

mod observer;
mod register;
mod bridge;
mod wrapper;
mod log;

ani_rs::ani_constructor! {
    namespace "L@ohos/telephony/observer/observer"
    [
        "onCellularDataFlowChange": observer::on_cellular_data_flow_change,
        "offCellularDataFlowChange": observer::off_cellular_data_flow_change,
        "onCellularDataFlowChangeOptions": observer::on_cellular_data_flow_change_option,
        "onIccAccountInfoChange": observer::on_icc_account_info_change,
        "offIccAccountInfoChange": observer::off_icc_account_info_change,
        "onSimStateChange": observer::on_sim_state_change,
        "onSimStateChangeOptions": observer::on_sim_state_change_option,
        "offSimStateChange": observer::off_sim_state_change,
        "onSignalInfoChange": observer::on_signal_info_change,
        "onSignalInfoChangeOptions": observer::on_signal_info_change_option,
        "offSignalInfoChange": observer::off_signal_info_change,
        "onCellInfoChange": observer::on_cell_info_change,
        "onCellInfoChangeOptions": observer::on_cell_info_change_option,
        "offCellInfoChange": observer::off_cell_info_change,
        "onCellularDataConnectionStateChange": observer::on_cellular_data_connection_state_change,
        "onCellularDataConnectionStateChangeOptions": observer::on_cellular_data_connection_state_change_option,
        "offCellularDataConnectionStateChange": observer::off_cellular_data_connection_state_change,
        "onNetworkStateChange": observer::on_network_state_change,
        "onNetworkStateChangeOptions": observer::on_network_state_change_option,
        "offNetworkStateChange": observer::off_network_state_change,
        "onCallStateChange": observer::on_call_state_change,
        "onCallStateChangeOptions": observer::on_call_state_change_option,
        "offCallStateChange": observer::off_call_state_change,
    ]
}

const LOG_LABEL: hilog_rust::HiLogLabel = hilog_rust::HiLogLabel {
    log_type: hilog_rust::LogType::LogCore,
    domain: 0xD001F07,
    tag: "StateRegistryJsApi",
};

#[used]
#[link_section = ".init_array"]
static G_TELEPHONY_PANIC_HOOK: extern "C" fn() = {
    #[link_section = ".text.startup"]
    extern "C" fn init() {
        std::panic::set_hook(Box::new(|info| {
            telephony_error!("Panic occurred: {:?}", info);
        }));
    }
    init
};
