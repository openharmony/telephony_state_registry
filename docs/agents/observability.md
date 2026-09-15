# 运维观测

← 返回 [`AGENTS.md`](../../AGENTS.md)

---

## 日志标识

| 项 | 值 | 定义位置 |
|---|---|---|
| 服务日志标签 | `StateRegistry` | 根目录 `BUILD.gn` 的 `defines` |
| 服务日志域 | `0xD001F07` | 根目录 `BUILD.gn` |
| 单元测试日志标签与域 | `StateRegistryTest` / `0xD000F00` | `test/unittest/state_test/BUILD.gn` |
| 进程 | `telecom` | `sa_profile/4009.json` |
| 系统服务 | 4009 | 同上 |

```shell
hdc shell hilog -D 0xD001F07
hdc shell hilog | grep -i StateRegistry
```

## dump

```shell
hdc shell hidumper -s 4009
```

`telephony_state_registry_dump_helper.cpp` 的输出包含两部分：各卡槽的状态缓存摘要，以及注册表 `registrations: count=` 与每条注册的 `package`、`pid`、`mask`、`slotId`。

每条注册只打印与其掩码匹配的**第一个**状态类别标签，一条注册同时订阅多个类别时，标签只反映其中之一，判断订阅内容以 `mask` 数值为准。

## 关键日志

| 日志片段 | 位置 | 判读 |
|---|---|---|
| `RegisterStateChange mask` | `RegisterStateChange` | 注册请求已通过全部校验并写入注册表 |
| `Register successfully, callback list size is` | `RegisterStateChange`，debug 级别 | 当前注册表大小，排查注册泄漏时对比增长趋势 |
| `Check permission failed` | `CheckPermission` 与各 `UpdateXxx` | 订阅方缺少对应权限，或上报方缺少 `SET_TELEPHONY_STATE` |
| `Non-system applications use system APIs` | `CheckCallerIsSystemApp` | 非系统应用订阅了系统级事件 |
| `VerifySlotId failed` | 各 `UpdateXxx` | 上报方传入的卡槽号越界 |
| `descriptor checked fail` | Stub | IPC 接口描述符不匹配，通常是跨部件版本不一致 |
| `OnRemoteRequest timeout func` | Stub 的 XCollie 回调 | 注册请求处理超过 30 秒 |
| `scope is nullptr`、`NapiReturnToJS callbackRef is nullptr` | `event_listener_handler.cpp` | JS 回调执行环境异常，关注其后回调是否仍能触发 |
| `listener is deleting` | `WorkUpdated` | 回调投递时监听已被取消，属于正常丢弃 |
| `AddStateObserver failed, ret=` | `RegisterEventListener` | JS 注册在 IPC 阶段失败，返回码见下表 |

## 返回码

| 返回码 | 含义 |
|---|---|
| `TELEPHONY_STATE_REGISTRY_PERMISSION_DENIED` | 权限不足 |
| `TELEPHONY_ERR_ILLEGAL_USE_OF_SYSTEM_API` | 非系统应用调用系统级订阅 |
| `TELEPHONY_STATE_REGISTRY_SLODID_ERROR` | 卡槽号非法，源码中的拼写即为 `SLODID` |
| `TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST` | 上报成功但没有匹配的观察者 |
| `TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST` | 取消注册时找不到对应记录 |
| `TELEPHONY_ERR_IPC_CONNECT_STUB_FAIL` | 客户端获取服务代理失败 |

JS 层把权限类失败转换为 `JS_ERROR_TELEPHONY_PERMISSION_DENIED` 抛出，参数类失败通过 `NapiUtil::ThrowParameterError` 抛出。

## XCollie 看护

根目录 `BUILD.gn` 在 hicollie 部件存在时定义 `HICOLLIE_ENABLE`。Stub 中只有 `ADD_OBSERVER` 接口码登记在 `collieCodeStringMap_` 里，超时阈值为 30 秒，其余上报接口不受看护。

## 排查顺序

1. 执行 `hidumper -s 4009`，确认目标应用是否在注册表中，以及掩码和卡槽是否符合预期。
2. 在服务日志中搜索 `Check permission failed` 与 `Non-system applications use system APIs`，排除权限原因。
3. 确认上报方是否在上报，搜索对应 `UpdateXxx` 的日志与返回码。
4. 服务侧正常而应用未收到回调时，转到应用进程日志，按「关键日志」表中 napi 相关条目排查。
5. 定位到模块后，转到 [`failure-modes.md`](failure-modes.md) 的「快速分诊表」。
