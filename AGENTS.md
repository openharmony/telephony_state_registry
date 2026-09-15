# telephony_state_registry 知识库入口

本文件是本仓知识库的唯一入口，其余文档位于 `docs/agents/`，全部从这里路由进入。

使用顺序：先读「这个仓是什么」确定方向，再用「知识路由表」选中文档，修改前核对「红线」，完成前执行「最小验证闭环」。

> **说明：**
>
> 本知识库描述 master 主干的状态。文档中的不变量是主干应当成立的约束，旧分支上某条不变量尚未落地时，缺口本身往往就是待修复的缺陷。读到一条不变量后，先在当前代码中搜索它的落点；找不到落点时，结论是「这里缺了一处」，而不是「文档写错了」；找到落点但写法与不变量相反时，结论相同。描述现状的表格（测试目标配置、文件清单、已知技术债）同样以 master 为准，其他分支上以实际代码为准。

---

## 这个仓是什么

OpenHarmony 电话子系统的**状态注册与分发**部件，`part_name` 为 `state_registry`。

- 运行形态：系统服务 SA 4009，进程 `telecom`，产物 `libtel_state_registry.z.so`。
- 职责：缓存通话、SIM、信号、网络、小区、蜂窝数据、呼转指示、语音信箱等状态，按订阅掩码与卡槽分发给观察者，并发布公共事件。
- 状态生产者：call_manager、core_service、cellular_data，通过 IPC 调用 `UpdateXxx`。
- 状态消费者：ArkTS 1.1 应用（napi）、ArkTS 1.2 应用（ani）、仓颉应用（cj）、系统部件（`TelephonyObserverClient`）。

心智模型：**一份按卡槽分组的状态缓存，加一张观察者注册表，再加三套前端各自维护的本地监听表。** 服务侧的缺陷多出在锁与生命周期，前端侧的缺陷多出在锁传递契约与回调负载的所有权。

主链路的完整图示见 [`docs/agents/code-map.md`](docs/agents/code-map.md)「三条主链路」，这里只列骨架：

```text
上报：生产者 → Stub → Service::UpdateXxx → 写缓存（独占锁）→ 遍历注册表回调（共享锁）→ 公共事件
注册：前端 on → 本地监听表 → TelephonyObserverClient → Service::RegisterStateChange → 注册表
回调：Service → 观察者 IPC → 前端 EventHandler → 本地监听表 → 应用回调
```

---

## 知识路由表

### 任务触发路由

| 任务 | 先读 | 再改 |
|---|---|---|
| 服务侧并发、崩溃、读到异常值 | [`boundaries.md`](docs/agents/boundaries.md)「服务侧锁约定」 | `services/src/telephony_state_registry_service.cpp` |
| 服务启动、异步任务、对象生命周期 | [`boundaries.md`](docs/agents/boundaries.md)「服务生命周期约束」 | 同上，`OnStart` 与所有异步任务 |
| JS 回调不触发、重复触发、死锁 | [`boundaries.md`](docs/agents/boundaries.md)「napi 前端的锁传递契约」 | `frameworks/js/napi/src/event_listener_handler.cpp` |
| 前端内存增长、泄漏、重复释放 | [`boundaries.md`](docs/agents/boundaries.md)「回调负载的所有权」 | napi 与 cj 两个 `*event_handler.cpp` |
| 新增一类可订阅状态 | [`boundaries.md`](docs/agents/boundaries.md)「新增一类订阅状态的清单」 | 服务、三套前端、声明文件，涉及 core_service 仓 |
| 新增或修改 IPC 接口 | [`boundaries.md`](docs/agents/boundaries.md)「IPC 定义在 core_service 仓」 | 跨仓改动，先停下确认 |
| 订阅权限、系统应用限制、号码可见性 | [`boundaries.md`](docs/agents/boundaries.md)「权限与隐私」 | `CheckPermission`、`CheckCallerIsSystemApp`、`TelephonyStateRegistryRecord` |
| 排查线上问题 | [`observability.md`](docs/agents/observability.md) | 无 |
| 按现象定位根因 | [`failure-modes.md`](docs/agents/failure-modes.md)「快速分诊表」 | 无 |
| 写用例、构建、验证 | [`build-and-test.md`](docs/agents/build-and-test.md) | `test/unittest/state_test/` |

### 路径触发路由

| 文件路径 | 必读知识 |
|---|---|
| `services/src/telephony_state_registry_service.cpp` | `boundaries.md`「服务侧锁约定」「服务生命周期约束」 |
| `services/src/telephony_state_registry_stub.cpp` | `boundaries.md`「IPC 定义在 core_service 仓」 |
| `services/src/telephony_state_registry_record.cpp` | `boundaries.md`「权限与隐私」 |
| `frameworks/js/napi/**` | `boundaries.md`「napi 前端的锁传递契约」「回调负载的所有权」 |
| `frameworks/cj/**` | `boundaries.md`「仓颉前端约束」「回调负载的所有权」 |
| `frameworks/ets/ani/**` | `boundaries.md`「新增一类订阅状态的清单」 |
| `frameworks/native/observer/**` | `boundaries.md`「模块边界与依赖方向」 |
| `interfaces/kits/js/*.d.ts` | 本文「必须停下来找人确认的改动」 |
| `BUILD.gn`、`bundle.json` | `build-and-test.md`「构建定位」 |
| `test/**` | `build-and-test.md`「测试目标」 |

### 词汇触发路由

| 词 | 含义 | 落点 |
|---|---|---|
| mask / `OBSERVER_MASK_*` | 订阅掩码，一位对应一类状态，定义在 core_service 的 `TelephonyObserverBroker` | 服务 `CheckPermission`、`UpdateData` |
| record | 注册表中的一条注册，以卡槽、掩码、tokenId、pid 四元组去重 | `stateRecords_` |
| notifyNow / isUpdate | 注册成功后立即补发一次当前缓存值 | `RegisterStateChange` → `UpdateData` |
| slotId `-1` | 不区分卡槽 | 状态缓存的键 |
| `SIM_SLOT_ID_FOR_DEFAULT_CONN_EVENT` | 值为 999 的特殊卡槽号，用于默认数据卡事件 | `RegisterStateChange` |
| `lock_` | 服务侧 `std::shared_mutex`，保护全部缓存与注册表 | 服务类成员 |
| `operatorMutex_` | 前端 EventHandler 的静态 `std::mutex`，保护本地监听表 | napi 与 cj 各一把 |
| `isDeleting` | 监听已取消的共享标志，排队中的回调据此丢弃 | `EventListener` |
| work / context | napi 侧投递到 JS 线程的 `uv_work_t` 与其携带的回调负载 | `HandleCallbackInfoUpdate` |
| CCallState | 跨设备通话状态，需要 `MANAGE_CALL_FOR_DEVICES` | 服务与前端 |
| ani | ArkTS 1.2 前端，Rust 桥接加 C++ | `frameworks/ets/ani/observer/` |
| cj / FFI | 仓颉前端 | `frameworks/cj/` |
| XCollie / `collieCodeStringMap_` | Stub 的 IPC 超时看护，只登记了 `ADD_OBSERVER`，阈值 30 秒 | `observability.md`「XCollie 看护」 |

### 动手前必须先自陈

写出第一行代码改动之前，先在回复中用三行写清楚：

```text
任务类别：<「任务触发路由」表中的哪一行>
已读文档：<docs/agents/ 下实际读过的文件与小节>
命中约束：<「红线」编号 + 分层文档中找到的不变量；确认没有则写"无">
```

这三行是检查点。写不出「命中约束」时，说明尚未掌握本次改动的影响范围，不要开始修改。

---

## 红线

违反以下任一条的改动不可合入。

1. **禁止不持锁访问状态缓存或 `stateRecords_`。** 写用 `std::unique_lock<std::shared_mutex>`，读用 `std::shared_lock`，查询类 `GetXxx` 同样适用。
2. **禁止在分离线程、延迟任务或异步回调中裸用 `this`。** 服务类继承了 `enable_shared_from_this`，异步体内使用 `weak_from_this()` 并在执行前 `lock()` 判空，需要的成员值先拷贝为局部变量。
3. **禁止打破 napi 前端的锁传递契约。** 接收 `std::unique_lock<std::mutex> &lock` 的函数，每一条返回路径都必须释放该锁，且调用 JS 回调前必须已释放。
4. **禁止持有 `operatorMutex_` 调用应用回调。** 应用在回调中调用 `on`/`off` 会重新获取同一把锁。
5. **禁止同一份回调负载存在两个所有者或零个所有者。** 分配方与释放方在代码中必须唯一且可指认，每条退出路径恰好释放一次。
6. **禁止在持有 `lock_` 的回调路径上反向调用注册或注销接口。** 观察者回调在持有共享锁期间发出，注册与注销需要独占锁，同线程重入会死锁。
7. **禁止把号码等隐私字段写入日志或未经 `READ_CALL_LOG` 校验返回给订阅者。**
8. **禁止只改一套前端。** 事件类型、回调字段的变化必须在 napi、ani、cj 三套前端与 `.d.ts`、`.ets` 声明中同步。
9. **禁止修改 `OBSERVER_MASK_*` 或 `StateNotifyInterfaceCode` 的已有取值。** 它们定义在 core_service 仓，是跨部件 ABI。
10. **禁止新增源文件而不同步根目录 `BUILD.gn` 或对应前端的 `BUILD.gn`。**

### 必须停下来找人确认的改动

碰到以下任一类，在最终回复中显式标注「需人工复核」并说明影响面：

| 触发条件 | 原因 |
|---|---|
| 修改 core_service 仓中的 IPC 接口、掩码或接口码 | 跨仓 ABI，本仓内无法验证 |
| 修改 `@ohos.telephony.observer.d.ts` 或 `.ets` 中已发布接口的签名或语义 | 应用兼容性 |
| 收紧或放宽任何权限校验、系统应用校验 | 两个方向都是安全事件 |
| 把「静默返回成功」的分支改为返回错误码 | 已发布行为变化，应用的错误处理会失效 |
| 改变注册表去重规则 | 影响所有订阅方的重复注册行为 |
| 修改 `OnStart`、`OnStop` 的时序 | 影响开机阶段所有生产者的首次上报 |
| 新增或替换外部部件依赖、第三方库（`external_deps`、`bundle.json` 的 `deps`） | 许可证与供应链合规审查，改变部件依赖关系 |

判据：这个改动的后果能不能在本仓内验证？不能，就属于本节。

---

## 最小验证闭环

详细命令见 [`docs/agents/build-and-test.md`](docs/agents/build-and-test.md)。

### 静态自检

必做，无需构建环境：

- 新增或修改的每个缓存成员，其全部读写点都在 `lock_` 保护下，读写锁类型正确。
- 每个异步体不捕获裸 `this`，不按引用捕获局部变量。
- 每个接收 `unique_lock &` 的函数，返回路径数与释放点一致。
- 每份回调负载的分配点与释放点成对，覆盖全部提前返回路径。
- 三套前端与两份声明文件已同步。
- `build-and-test.md`「无构建环境时的静态自检」逐项通过。

### 构建与测试

```shell
./build.sh --product-name <product> --build-target state_registry
./build.sh --product-name <product> --build-target "//base/telephony/state_registry/test:unittest"
hdc shell /data/test/tel_state_registry_branch_test --gtest_filter=<用例名>*
```

### DoD 判定

- [ ] 目标行为在最小复现场景下被验证，场景写入提交说明。
- [ ] 至少一条带实际断言的用例覆盖改动分支；前端代码无单测时，写明替代验证方式。
- [ ] 并发类改动已说明读写交错场景，生命周期类改动已说明对象销毁后的执行路径。
- [ ] 日志中无号码等隐私字段。

### 最终回复必须包含

1. 改了什么：文件清单，每处一句话说明为什么改在这里。
2. 「动手前必须先自陈」中的三行。
3. DoD 逐项结果：已验证或未验证，未验证写明原因。
4. 验证边界：没有 OpenHarmony 构建环境时，明说「仅完成静态自检，未编译、未运行测试」。
5. 遗留风险：需人工复核的项、发现但未修的技术债、同族代码中未一并处理的相同模式。

---

## 分层文档索引

| 文档 | 内容 | 什么时候读 |
|---|---|---|
| [`docs/agents/code-map.md`](docs/agents/code-map.md) | 目录职责、三条主链路、关键类、任务到路径 | 第一次进入某个模块 |
| [`docs/agents/boundaries.md`](docs/agents/boundaries.md) | 依赖方向、锁约定、生命周期、锁传递契约、所有权、权限、变更清单 | 动手改代码前必读 |
| [`docs/agents/failure-modes.md`](docs/agents/failure-modes.md) | 快速分诊表、失效模式、根因排查方法、修复反例、回归要求 | 排查缺陷时 |
| [`docs/agents/observability.md`](docs/agents/observability.md) | 日志标识、dump、关键日志、返回码 | 定位现场问题 |
| [`docs/agents/build-and-test.md`](docs/agents/build-and-test.md) | 构建目标、测试目标、静态自检、DoD | 验证阶段 |
