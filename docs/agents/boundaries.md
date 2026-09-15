# 边界与约束

← 返回 [`AGENTS.md`](../../AGENTS.md)

---

## 模块边界与依赖方向

```text
interfaces/kits (d.ts / ets)
      │ 声明
frameworks/js/napi  frameworks/ets/ani  frameworks/cj      三套前端，互不依赖
      └──────────────┬───────────────┘
                     ▼
frameworks/native/observer      TelephonyObserverClient / TelephonyStateManager
                     │ IPC（接口定义来自 core_service）
                     ▼
services/src  Stub → Service → Record
```

| 规则 | 说明 |
|---|---|
| 前端之间不互相引用 | 三套前端各自维护本地监听表与锁，修复一套时逐一检查另外两套是否存在相同模式 |
| 前端不包含 `services/` 头文件 | 前端只通过 `TelephonyStateManager` 与服务交互 |
| Stub 不写业务逻辑 | Stub 只校验描述符、反序列化、转发 |
| 服务侧 `telephony_observer_proxy.cpp` 编进服务库 | 它是服务回调观察者的代理，不是客户端代码 |

### IPC 定义在 core_service 仓

`ITelephonyStateNotify`、`TelephonyObserverBroker`、`StateNotifyInterfaceCode`、`TelephonyStateRegistryProxy` 不在本仓，随 `core_service:tel_core_service_api` 引入。

- 新增 IPC 接口需要两仓配合：core_service 仓追加接口码与代理方法，本仓 Stub 在 `memberFuncMap_` 中登记处理函数。
- 接口码只能在枚举末尾追加，已有取值禁止改动。
- Stub 中登记了 XCollie 看护的只有 `ADD_OBSERVER`，新增耗时接口时同步登记到 `collieCodeStringMap_`。
- 本仓无法独立验证 IPC 改动，属于「必须停下来找人确认的改动」。

---

## 服务侧锁约定

`TelephonyStateRegistryService::lock_` 是 `std::shared_mutex`，保护**全部**状态缓存 map 与 `stateRecords_`。

| 场景 | 锁 |
|---|---|
| 写缓存（`UpdateXxx` 中的赋值） | `std::unique_lock<std::shared_mutex>` |
| 遍历注册表并回调观察者 | `std::shared_lock<std::shared_mutex>` |
| 查询缓存（`GetXxx`、`UpdateData`、dump） | `std::shared_lock<std::shared_mutex>` |
| 增删注册（`RegisterStateChange`、`UnregisterStateChange`） | `std::unique_lock<std::shared_mutex>` |

不变量与检查方法：

- **每个缓存成员的每个读点都在锁内。** 检查方法：对成员名全文搜索，逐个确认所在函数已持锁；不要只核对与现象相关的几个函数。`std::map::operator[]` 会插入节点，与并发遍历交错即为数据竞争。
- **`UpdateXxx` 先独占写、释放、再共享遍历。** 两段之间有窗口，遍历时读到的缓存值允许已被后续写入更新，这是现有设计，不要为此把回调放进独占锁。
- **观察者回调在共享锁内发出。** 同进程内的系统部件观察者若在回调中同步调用注册或注销，会在同线程等待独占锁而死锁。新增回调路径时不要引入这类重入。
- **非递归。** 已持锁的函数内不要调用另一个自行加锁的公开函数，`std::shared_mutex` 不支持递归加锁，同一线程重复获取共享锁同样是未定义行为，有写者等待时会死锁。
- **已知技术债：`Dump` 路径重复获取共享锁。** `TelephonyStateRegistryService::Dump` 持有共享锁调用 `TelephonyStateRegistryDumpHelper::Dump`，后者经单例回调 `GetSimState`、`GetCallState` 等，这些查询函数再次获取共享锁。修改查询函数或 dump 时，先在锁内拷贝 `stateRecords_` 快照，释放锁后再调用 dump helper。给查询函数补锁时必须同时处理这条路径。

新增缓存成员清单：

1. 头文件声明，写明由 `lock_` 保护。
2. 对应 `UpdateXxx` 在独占锁内写。
3. `UpdateData` 与 `GetXxx` 在共享锁内读。
4. dump 输出在共享锁内读。
5. 全文搜索成员名，确认没有遗漏的读点。

---

## 服务生命周期约束

| 事实 | 约束 |
|---|---|
| 服务类由 `DECLARE_DELAYED_SINGLETON` 管理，继承 `SystemAbility` 与 `enable_shared_from_this` | 取自身强引用用 `shared_from_this()`，取弱引用用 `weak_from_this()` |
| `OnStart` 创建分离线程 `state_registry_task` | 分离线程的执行时间与服务对象的存续时间没有同步关系 |
| 服务可被 samgr 停止与重新拉起 | 异步体不能假设执行时对象仍然有效 |

异步体的写法约束：

- 捕获 `weak_from_this()` 返回的弱引用，执行时 `lock()`，结果为空立即返回。
- 需要使用的成员值（例如卡槽数量）在创建异步体前拷贝为局部变量并按值捕获。
- 禁止 `[&]` 或 `[this]` 捕获后分离执行。`[&]` 在成员函数内同样隐式捕获 `this` 指针。
- 用强引用捕获会延长服务对象寿命，跨过 `OnStop`，只有确实需要保活时才使用，并在回复中说明理由。

---

## napi 前端的锁传递契约

`EventListenerHandler::operatorMutex_` 是静态 `std::mutex`，保护 `listenerList_`。

回调执行链：

```text
WorkUpdated（JS 线程）
  创建 std::unique_lock<std::mutex> lock(operatorMutex_)
  └─ workFuncMap_[eventType](work, lock)        WorkXxxUpdated
       └─ NapiReturnToJS(env, callbackRef, value, lock)
            正常路径：取回调函数 → lock.unlock() → napi_call_function
```

契约：

- **锁以引用方式向下传递，由被调用方负责在调用 JS 前释放。** 调用方在函数返回后不再假设锁的状态。
- **每一条返回路径都必须释放锁，与正常路径保持一致。** 提前返回（句柄作用域打开失败、回调引用为空、回调函数为空、参数异常）同样要 `lock.unlock()`。
- **检查方法：** 对每个接收 `unique_lock &` 的函数，列出全部 `return` 语句，逐个确认之前有 `unlock()`；再确认 `napi_call_function` 之前已释放。不要用「同族函数多数怎么写」来判断对错，以本契约为准。
- **句柄作用域成对。** `napi_open_handle_scope` 成功后，每条路径都要 `napi_close_handle_scope`；打开失败时不要关闭。
- **`isDeleting` 在持锁状态下判断。** `off` 会置位共享标志，排队中的回调在 `WorkUpdated` 中据此丢弃。

`HandleCallbackInfoUpdate` 运行在 `EventListenerHandler` 自己的事件线程上，持锁遍历监听表并通过 `uv_queue_work_with_qos` 投递，投递失败时释放本次分配的负载与 work。

---

## 回调负载的所有权

两套前端的分发模型不同，所有权约定也不同，**不要把一套的写法搬到另一套。**

| 前端 | 分发方式 | 负载所有者 |
|---|---|---|
| napi | 异步：投递 `uv_work_t` 到 JS 线程 | 投递成功后所有权随 `work->data` 转交，由 `WorkUpdated` 或 `WorkXxxUpdated` 释放；投递失败由投递方释放 |
| cj | 同步：事件线程上直接调用回调 | 分配方在分发返回后释放，`WorkXxxUpdated` 只借用，不接管 |

不变量：

- 每份负载恰好一个所有者，所有者在**每条退出路径**上恰好释放一次。
- 被调函数用 `std::unique_ptr` 接管负载，意味着它是所有者；此时它的每个提前返回都必须发生在接管之后，否则泄漏。
- 分配方与被调方同时释放即为重复释放；两方都不释放即为泄漏。
- 检查方法：从分配语句出发，沿所有分支追踪到释放语句，覆盖 `isDeleting` 丢弃、事件类型未知、回调为空、参数为空、投递失败等分支。
- 判定规则：**若**同步分发的前端（cj）中某个 `WorkXxxUpdated` 用 `std::unique_ptr` 接管负载，**则**违反该前端的借用约定。这种写法正常路径看起来释放了一次，丢弃分支与未接管的处理函数上仍然泄漏；修复时把释放收拢到分配方，并把所有 `WorkXxxUpdated` 改为借用，两处必须同时改。master 上 cj 前端已是借用写法；napi 前端的 `WorkXxxUpdated` 用 `std::unique_ptr` 接管属于异步分发的正确写法，不适用本条。

---

## 仓颉前端约束

- `ObserverEventHandler` 以快照方式分发：持 `operatorMutex_` 拷贝匹配的监听，释放锁后逐个回调。新增分发路径保持这一模式，不在持锁状态下调用仓颉回调。
- 回调参数中的字符串由 `MallocCString` 分配，所有权交给仓颉侧；结构体数组由 `malloc` 分配，分配失败与提前返回时释放已分配部分。
- FFI 导出函数的指针参数一律判空。

---

## 权限与隐私

订阅权限由服务侧 `CheckPermission(mask)` 与 `CheckCallerIsSystemApp(mask)` 判定：

| 掩码 | 要求 |
|---|---|
| `OBSERVER_MASK_NETWORK_STATE` | `GET_NETWORK_INFO` |
| `OBSERVER_MASK_CELL_INFO` | `CELL_LOCATION`，且需系统应用 |
| `OBSERVER_MASK_CCALL_STATE` | `MANAGE_CALL_FOR_DEVICES` |
| `OBSERVER_MASK_SIM_ACTIVE_STATE` | `SET_TELEPHONY_STATE` |
| 呼转指示、语音信箱指示 | 系统应用 |
| 全部 `UpdateXxx` | 调用方需 `SET_TELEPHONY_STATE` |

隐私与兼容约束：

- 来电号码只对具有 `READ_CALL_LOG` 的订阅者可见，判断在 `TelephonyStateRegistryRecord` 中完成。新增携带号码的回调必须经过同一判断。
- `RegisterStateChange` 对多卡能力不支持、卡槽越界两种情况**静默返回成功**。这是已发布行为，订阅方表现为「注册成功但收不到回调」，排查时优先检查卡槽号，不要直接改为返回错误。
- 日志中不打印号码。

---

## 新增一类订阅状态的清单

1. core_service 仓：追加 `OBSERVER_MASK_*`、观察者接口方法、接口码（跨仓，先确认）。
2. 服务：新增缓存成员（按「服务侧锁约定」清单）、`UpdateXxx`、`UpdateData` 补发分支、`CheckPermission` 权限映射、公共事件（如需要）、dump 标签。
3. 服务侧观察者代理：`frameworks/native/observer/src/telephony_observer_proxy.cpp`。
4. 客户端：`TelephonyObserver` 的 IPC 分发与 `OnXxxUpdated`。
5. napi：事件类型、更新信息结构、`HandleCallbackInfoUpdate` 分发登记、`WorkXxxUpdated`（遵守锁传递契约与所有权约定）。
6. ani：`.ets` 声明、Rust 注册与桥接、C++ 实现。
7. cj：`ObserverEventHandler` 分发分支、FFI 结构体。
8. 声明：`@ohos.telephony.observer.d.ts` 补齐 `@permission`、`@throws`、`@syscap`、`@since`。
9. 测试：`state_registry_branch_test` 覆盖权限拒绝与正常路径。
