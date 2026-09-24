# 编译与测试

← 返回 [`AGENTS.md`](../../AGENTS.md)

---

## 构建定位

本仓是 OpenHarmony 的一个部件，不能独立构建，源码树中的位置为 `<OHOS_ROOT>/base/telephony/state_registry`。

| 项 | 值 |
|---|---|
| `part_name` | `state_registry` |
| `subsystem_name` | `telephony` |
| 服务库 | `//base/telephony/state_registry:tel_state_registry` |
| 客户端库 | `//base/telephony/state_registry/frameworks/native/observer:tel_state_registry_api` |
| ArkTS 1.1 模块 | `//base/telephony/state_registry/frameworks/js/napi:observer` |
| ArkTS 1.2 模块 | `//base/telephony/state_registry/frameworks/ets/ani/observer:observer_ani_group` |
| 仓颉 FFI | `//base/telephony/state_registry/frameworks/cj:cj_observer_ffi` |
| 测试组 | `test:unittest`、`test/fuzztest:fuzztest` |

`bundle.json` 的 `features` 为空，本仓没有部件级 feature 开关。

### 构建宏

| 宏 | 条件 | 影响 |
|---|---|---|
| `HICOLLIE_ENABLE` | hicollie 部件存在 | Stub 的 XCollie 看护 |
| `OHOS_BUILD_ENABLE_TELEPHONY_EXT`、`OHOS_BUILD_ENABLE_TELEPHONY_VSIM` | `telephony_telephony_enhanced` 为真 | 扩展库加载与虚拟卡相关分支 |

宏内声明的成员，其引用点必须在同一宏内，宏关闭时仍要能编过。

### 编译选项

服务库开启 `cfi`、`cfi_cross_dso`、`branch_protector_ret = "pac_ret"`，编译选项含 `-O2` 与 `-D_FORTIFY_SOURCE=2`。跨 DSO 的函数指针转换会被 CFI 在运行期拦截，扩展库调用处保持现有写法。

### 静态检查工具

本仓没有部件内的 lint、格式化配置或静态分析脚本（无 `.clang-format`、`.clang-tidy`、`cpplint` 配置）。代码风格与静态检查由 OpenHarmony 门禁流水线执行，本地以编译告警与本文「无构建环境时的静态自检」替代，不要查找不存在的本地 lint 命令。

### 构建命令

```shell
# 整个部件
./build.sh --product-name <product> --build-target state_registry --ccache

# 服务库
./build.sh --product-name <product> --build-target "//base/telephony/state_registry:tel_state_registry"

# 全部单测
./build.sh --product-name <product> --build-target "//base/telephony/state_registry/test:unittest"

# 单个测试目标
./build.sh --product-name <product> \
  --build-target "//base/telephony/state_registry/test/unittest/state_test:tel_state_registry_branch_test"
```

`<product>` 取决于开发板，常见 `rk3568`。

---

## 测试目标

| 目标 | 源文件 | 特点 | 适用 |
|---|---|---|---|
| `tel_state_registry_test` | `state_registry_test.cpp` | 链接服务库与客户端库，无替身，走真实权限校验 | 接口正常路径、注册注销 |
| `tel_state_registry_branch_test` | `state_registry_branch_test.cpp` + `test/mock/mock_telephony_permission.cpp` | 权限校验被替身替换，带 gmock | 权限拒绝、系统应用限制、分支覆盖，**服务侧修复首选** |

注意事项：

- 两个目标的 `module_out_path` 不同：`tel_state_registry_test` 为 `state_registry/state_registry/tel_state_registry_test`，`tel_state_registry_branch_test` 为 `state_registry/tel_state_registry_test`。
- 两个目标都没有链接 `ffrt_mocked`。两个测试文件都在开头 `#define private public` 与 `#define protected public`，新增用例沿用这一写法访问私有成员。
- **napi、ani、cj 三套前端没有单元测试。** 前端改动只能做静态自检与设备验证，最终回复中必须写明。
- 模糊测试有两个目标：`TelephonyObserverFuzzTest`、`TelephonyStateRegistryFuzzTest`。`test/fuzztest/common_fuzzer/` 只提供授权辅助代码，不是独立目标。新增模糊测试目录后，必须在 `test/fuzztest/BUILD.gn` 中登记。

### 运行

```shell
hdc shell /data/test/tel_state_registry_branch_test
hdc shell /data/test/tel_state_registry_branch_test --gtest_filter=<Suite>.<Case>
hdc shell /data/test/tel_state_registry_branch_test --gtest_list_tests
```

### 用例编写约定

- 用例前三行注释：`@tc.number`、`@tc.name`、`@tc.desc`。
- 服务是单例，用例改过的缓存与注册表要在 `TearDown` 中恢复。
- 必须包含实际断言，断言被修复的具体状态或返回码，不要只验证不崩溃。

---

## 无构建环境时的静态自检

| # | 检查 | 方法 |
|---|---|---|
| 1 | 新增源文件已进对应 `BUILD.gn` 的 `sources` | 搜索文件名 |
| 2 | 服务侧每个缓存成员的读写点都持锁，锁类型正确 | 搜索成员名，逐个核对 |
| 3 | 异步体没有裸 `this` 或 `[&]` 捕获 | 搜索 `std::thread`、`detach`、`submit`、`PostTask` |
| 4 | 接收 `unique_lock &` 的函数，每条返回路径都释放锁 | 逐个列出 `return` |
| 5 | 调用 JS 或仓颉回调时不持有 `operatorMutex_` | 核对回调调用点之前的锁状态 |
| 6 | 每份回调负载的分配与释放成对，覆盖丢弃分支 | 从分配语句追踪全部分支 |
| 7 | `napi_open_handle_scope` 与 `napi_close_handle_scope` 成对 | 逐条路径核对 |
| 8 | FFI 导出函数的指针参数判空 | 核对函数入口 |
| 9 | 三套前端与 `.d.ts`、`.ets` 已同步 | 搜索事件名 |
| 10 | 新增携带号码的回调经过 `READ_CALL_LOG` 判断，日志不含号码 | 核对 `TELEPHONY_LOG*` |
| 11 | 宏内成员只在宏内引用 | 搜索成员名 |

---

## DoD

完整 DoD 见 [`AGENTS.md`](../../AGENTS.md)「最小验证闭环」。最小要求：

- [ ] 本文静态自检全部通过。
- [ ] 有构建环境时，`state_registry` 部件与两个单测目标编译通过并运行全绿。
- [ ] 服务侧改动至少一条带断言的用例覆盖；前端改动写明设备验证步骤。
- [ ] [`failure-modes.md`](failure-modes.md)「回归要求」中的对应项已确认。
