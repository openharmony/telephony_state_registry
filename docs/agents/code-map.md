# 代码地图

← 返回 [`AGENTS.md`](../../AGENTS.md)

---

## 部件定位

| 项 | 值 |
|---|---|
| 部件 | `state_registry`，子系统 `telephony` |
| 系统服务 | SA 4009，进程 `telecom`，产物 `libtel_state_registry.z.so`，`run-on-create=true`（`sa_profile/4009.json`） |
| 职责 | 缓存通话、SIM、信号、网络、小区、蜂窝数据、呼转指示、语音信箱等电话状态，按订阅掩码与卡槽分发给观察者，并发布对应公共事件 |
| 状态生产者 | call_manager、core_service、cellular_data 等部件，通过 IPC 调用 `UpdateXxx` 接口上报 |
| 状态消费者 | 应用（`@ohos.telephony.observer`）、仓颉应用、系统部件（通过 `TelephonyObserverClient` 注册）、公共事件订阅方 |

心智模型：本仓是**一份按卡槽分组的状态缓存，加一张观察者注册表**。上报时先写缓存，再遍历注册表回调；注册时若要求立即通知，则把当前缓存值补发一次。

## 非本仓维护的定义

以下内容不在本仓代码树中，随 `core_service:tel_core_service_api` 依赖引入。修改它们属于跨仓改动：

| 定义 | 说明 |
|---|---|
| `ITelephonyStateNotify` | 状态上报与注册的 IPC 接口 |
| `TelephonyObserverBroker` | 观察者 IPC 接口，含全部 `OBSERVER_MASK_*` 订阅掩码 |
| `StateNotifyInterfaceCode` | 服务侧 IPC 接口码枚举 |
| `TelephonyStateRegistryProxy` | 生产者与客户端使用的服务代理 |

本仓内只有观察者方向的代理 `frameworks/native/observer/src/telephony_observer_proxy.cpp`，它被编进服务库，用于服务回调观察者。

## 嵌套指引

本仓没有目录级的 `AGENTS.md`。全部任务指引经由根目录 `AGENTS.md` 路由到 `docs/agents/` 下的分层文档。

## 目录职责

```text
telephony_state_registry/
├── BUILD.gn                 服务库 tel_state_registry 的构建定义与宏
├── bundle.json              部件声明与构建分组
├── sa_profile/              SA 4009 启动配置
├── services/
│   ├── include/ src/        服务、IPC 分发、注册记录、dump
│   └── telephony_ext_wrapper/  扩展库动态加载
├── frameworks/
│   ├── native/observer/     客户端注册入口与观察者 IPC 实现
│   ├── js/napi/             ArkTS 1.1 的 observer 模块
│   ├── ets/ani/observer/    ArkTS 1.2 的 observer 模块，Rust 桥接加 C++ ANI
│   └── cj/                  仓颉 FFI
├── interfaces/
│   ├── innerkits/observer/  对系统部件公开的观察者头文件
│   └── kits/js/             对应用公开的 d.ts 声明
└── test/
    ├── unittest/state_test/ 服务侧单元测试
    ├── fuzztest/            两个模糊测试目标
    └── mock/                权限校验替身
```

## 三条主链路

### 上报链路

```text
生产者部件（call_manager / core_service / cellular_data）
  └─ TelephonyStateRegistryProxy（core_service 提供）
       └─ IPC → services/src/telephony_state_registry_stub.cpp
            OnRemoteRequest：校验接口描述符 → memberFuncMap_ 分发 → OnUpdateXxx 反序列化
            └─ services/src/telephony_state_registry_service.cpp
                 UpdateXxx：卡槽校验 → 权限校验 → 独占锁写缓存 → 共享锁遍历 stateRecords_
                 ├─ 按掩码与卡槽回调 record.telephonyObserver_->OnXxxUpdated
                 └─ SendXxxChanged 发布公共事件
```

### 注册链路

```text
应用 observer.on(...)
  └─ frameworks/js/napi/src/napi_state_registry.cpp          On → NativeOn
       └─ event_listener_manager.cpp → event_listener_handler.cpp  RegisterEventListener
            同卡槽同事件类型已有注册时只追加本地监听，不重复发起 IPC
            └─ frameworks/native/observer/src/telephony_state_manager.cpp
                 └─ telephony_observer_client.cpp  GetProxy（带死亡监听）→ RegisterStateChange
                      └─ IPC → Stub::OnRegisterStateChange → Service::RegisterStateChange
                           系统应用校验 → 权限校验 → 多卡能力校验 → 卡槽范围校验
                           → 写入 stateRecords_ → notifyNow 为真时 UpdateData 补发当前值
```

### 回调链路（应用进程内）

```text
服务调用观察者代理 → 应用进程 TelephonyObserver::OnRemoteRequest
  └─ napi_telephony_observer.cpp  OnXxxUpdated 构造更新信息 → EventListenerManager::SendEvent
       └─ EventListenerHandler（独立 EventRunner 线程）HandleCallbackInfoUpdate
            持 operatorMutex_ 遍历 listenerList_ → uv_queue_work_with_qos 投递到 JS 线程
            └─ WorkUpdated（JS 线程，持 operatorMutex_）→ WorkXxxUpdated → NapiReturnToJS 调用 JS 回调
```

仓颉侧的结构与之平行：`frameworks/cj/src/observer_event_handler.cpp` 中的 `ObserverEventHandler` 承担 `EventListenerHandler` 的角色。

## 关键类与数据

| 类或数据 | 位置 | 说明 |
|---|---|---|
| `TelephonyStateRegistryService` | `services/src/telephony_state_registry_service.cpp` | 单例服务，持有全部状态缓存与注册表 |
| `lock_` | 同上，`std::shared_mutex` | 保护全部缓存 map 与 `stateRecords_` |
| 状态缓存 | `callState_`、`simState_`、`signalInfos_`、`searchNetworkState_`、`cellularDataConnectionState_` 等 | 键为卡槽号，`-1` 表示不区分卡槽 |
| `stateRecords_` | 同上，`std::vector<TelephonyStateRegistryRecord>` | 注册表，以卡槽、掩码、tokenId、pid 四元组去重 |
| `TelephonyStateRegistryRecord` | `services/src/telephony_state_registry_record.cpp` | 单条注册，含调用方身份与观察者代理，提供读通话记录与跨设备管理权限查询 |
| `TelephonyStateRegistryStub` | `services/src/telephony_state_registry_stub.cpp` | IPC 分发；仅 `ADD_OBSERVER` 配置了 XCollie 超时看护 |
| `EventListenerHandler` | `frameworks/js/napi/src/event_listener_handler.cpp` | JS 侧监听表与回调投递，静态锁 `operatorMutex_` |
| `ObserverEventHandler` | `frameworks/cj/src/observer_event_handler.cpp` | 仓颉侧监听表与回调投递 |
| `TelephonyExtWrapper` | `services/telephony_ext_wrapper/` | `OHOS_BUILD_ENABLE_TELEPHONY_EXT` 宏打开时加载扩展库 |

## 任务到路径

| 任务 | 首先打开 |
|---|---|
| 新增一类可订阅状态 | `services/src/telephony_state_registry_service.cpp` 的 `UpdateXxx`、`UpdateData`、`CheckPermission`，以及 napi、ani、cj 三个前端；掩码定义在 core_service |
| 调整订阅权限或系统应用限制 | `CheckPermission`、`CheckCallerIsSystemApp`、`IsMultiSimsCapabilitySupported` |
| 修复服务侧并发或生命周期问题 | `telephony_state_registry_service.cpp` 中所有访问缓存与 `stateRecords_` 的函数，以及 `OnStart` |
| 修复 JS 回调不触发、重复触发、崩溃 | `frameworks/js/napi/src/event_listener_handler.cpp` |
| 修复仓颉回调或内存问题 | `frameworks/cj/src/observer_event_handler.cpp` |
| 修复 ArkTS 1.2 接口 | `frameworks/ets/ani/observer/` 下的 `.ets`、Rust 源码与 `observer_ani.cpp` |
| 修改 dump 输出 | `services/src/telephony_state_registry_dump_helper.cpp` |
| 补充单元测试 | `test/unittest/state_test/` |

## 文件体量

下表为快照值，用于判断相对量级，不作为可校验断言。统计口径为含空行的行数。

| 文件 | 量级 |
|---|---|
| `test/unittest/state_test/state_registry_test.cpp` | ~1400 |
| `frameworks/js/napi/src/event_listener_handler.cpp` | ~1100 |
| `services/src/telephony_state_registry_service.cpp` | ~1060 |
| `interfaces/kits/js/@ohos.telephony.observer.d.ts` | ~940 |
| `frameworks/cj/src/observer_event_handler.cpp` | ~630 |
| `frameworks/js/napi/src/napi_state_registry.cpp` | ~550 |
| `services/src/telephony_state_registry_stub.cpp` | ~460 |
