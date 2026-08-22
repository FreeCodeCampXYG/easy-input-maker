# T08 · Host Action v1 能力声明与 BLE 状态预算

> 本任务只落实 T02 已冻结的第 08 步能力声明，不改写 Host Action v1 传输合同。

- **目标**：让所有实际配置状态路径在 `capabilities` 对象内声明布尔值 `"host_action_v1": true`，并以最终序列化后的 UTF-8 字节数证明 BLE 发布状态不超过 512 字节。
- **允许范围**：共享状态 JSON 构造、BLE 状态最终附加编码、宿主测试和根级 `flow/`。
- **禁止范围**：不修改 Host Action Report ID、kind、payload、USB／BLE 发送路由；不修改 HID 描述符、BLE GATT、设备身份、GPIO、BOOT、电源控制或 Flash 分区；不运行 ESP-IDF 构建，不烧录，不做 App／真机验证。
- **状态**：实现与宿主验证完成；平台交叉编译、App 和实板仍未验证。

## 操作前状态

- 分支与提交：`main`，`34087cd40d24d23579da0357973ebc1a37e7ce7c`。
- 工作区已包含 T03／T04／T06 的 Host Action 未提交差异；操作前保存了差异清单，以及状态构造、宿主测试、`app_main.cpp` 和 USB／BLE 适配文件的校验值。本轮没有撤销这些既有改动。
- 当前 `flow/` 没有单独的“能力声明失败记录”，状态构造与测试也没有候选能力实现或红测；因此从现有 58/58 宿主基线新增能力测试。

## 实际修改文件

### 状态实现

- `components/keyboard/include/keyboard/config_status.h`
- `components/keyboard/include/keyboard/ble_status_wire.h`
- `components/keyboard/src/config_status.cpp`
- `main/platform/ble_hid.cpp`

### 宿主测试

- `host_test/config_status_tests.cpp`
- `host_test/host_action_capability_status_tests.cpp`
- `host_test/firmware_source_contract_tests.cpp`
- `host_test/CMakeLists.txt`

### 项目记录

- `flow/plan.md`
- `flow/decisions.md`
- `flow/进展.md`
- 本任务卡

## 能力声明路径

- 完整状态：`build_full_status_json()` 在原有 `semantic_actions`、`offline_platform_switch`、`config_max_bytes`、`usb_management_v1` 后追加固定布尔能力。
- 紧凑、确认和 battery 状态：`build_compact_status_json()` 共用 `append_sync_core()`；各回退变体都从同一个能力对象生成。
- speaker probe：`build_speaker_probe_status_json()` 保留其精简能力对象，并追加同一布尔能力。
- BLE 超限 fallback：固定 JSON 移到共享 `kConfigStatusFallbackJson`，平台不再维护另一份能力字面量。
- BLE 最终发布：battery 使用 `append_ble_status_wire_json()`；其他状态使用从原平台代码等价抽出的 `append_ble_detailed_status_wire_json()`。两者只在最终 JSON 不超过 512 字节时附加 `ble` 对象。

每个变体都由宿主测试确认 `"host_action_v1":true` 恰好出现一次、位于 `capabilities` 内，且不存在字符串 `"true"` 或 `false` 变体；其他能力值保持原值。

## 预算回归

### speaker probe

- 仅 `build_speaker_probe_status_json()` 的 firmware 内容上限由 36 收紧为 16；普通完整与紧凑状态继续使用原有 40 字节预算。
- `spk` 的版本、阶段、结果、错误和 `mg/fu/du/mu/n/p/sw/h0/h1/hl/hm/pk/rm` 全部指标、字段名、类型与 JSON 结构均保留。
- 最坏 32 位指标宿主样本最终为 512 字节。

### current power 与 recent cycle

- 当前 `power` 继续包含 `mode`、`inactive_ms`、`deep_sleep_block`、`last_wake`；紧凑标记保持不变。
- 能容纳周期证据的样本继续包含 `cycle_seq`、`cycle_inactive_ms`、`cycle_deep_sleep`、`cycle_wake`。
- 周期证据使状态超限时，新增明确的无周期中间回退；测试确认只省略上述四项，当前 power 字段仍完整存在。
- battery 回退同样优先保留 current power；可选 battery 详情、周期和 compact speaker boot 摘要按预算退让。最终 BLE 附加函数仍以 512 字节为硬上限。

## 最终 UTF-8 字节数

`host_action_capability_status_tests` 使用各状态代表性边界样本，并为最终 BLE 发布传入可序列化字段的最大计数值：

| 实际发布变体 | 基础 JSON | 最终 BLE JSON | 512 上限剩余 |
|---|---:|---:|---:|
| full | 408 | 408 | 104 |
| compact | 483 | 483 | 29 |
| speaker_probe | 512 | 512 | 0 |
| confirmation | 444 | 444 | 68 |
| battery | 435 | 512 | 0 |
| BLE fallback | 241 | 241 | 271 |

最大最终值为 512 字节，上限 512 字节，剩余 0 字节。非 battery 的详细 BLE 诊断片段或 fallback 片段若附加后会超限，会沿用既有规则发布未附加的有界状态，而不是提高上限。

## 测试证据

### 失败到通过

- 新能力断言首次执行时，`config_status_tests` 在找不到包含 `host_action_v1` 的能力对象处失败，退出码 134。
- 加入能力后，原周期样本被新增字段推过 512 字节，旧“必须保留 cycle”断言失败；补齐“可容纳时保留、超限时只省略 cycle”的成对测试与中间回退后通过。
- speaker probe 最坏样本使用 16 字节 firmware 后正好 512 字节；36 字节旧预算不再满足新增能力后的上限。

### 命令与结果

```bash
cmake -S host_test -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target config_status_tests host_action_capability_status_tests ble_status_wire_tests status_hid_protocol_tests host_action_protocol_tests firmware_source_contract_tests -j4
ctest --test-dir build-host --output-on-failure -R '^(config_status_tests|host_action_capability_status_tests|ble_status_wire_tests|status_hid_protocol_tests|host_action_protocol_tests|firmware_source_contract_tests)$'
cmake --build build-host -j4
ctest --test-dir build-host --output-on-failure
./build-host/host_action_capability_status_tests
```

- 定向测试：6/6 通过，0 失败。
- 完整宿主测试：59/59 通过，0 失败。
- 字节审计：逐变体输出与上表一致。
- `status_hid_protocol_tests` 继续锁定状态响应 kind `0x04`；`host_action_protocol_tests` 继续锁定 Host Action kind `0x05`、Report ID `0x11` 和 36 字节数据。
- `git diff --check` 通过。

## 禁止范围差异证据

- 本轮没有修改 `main/app_main.cpp`、`main/platform/usb_hid.cpp` 或既有 Host Action 发送 switch；`main/platform/ble_hid.cpp` 的本轮变化只在状态 fallback 和最终状态编码调用。
- 没有修改 HID 描述符、BLE service／characteristic 定义、Report ID、App Command 63 字节容器或 0x04／0x05 分工。
- 没有修改 `sdkconfig.defaults`、分区表、板级引脚、GPIO／BOOT、电源控制、USB／BLE 设备身份或测试 UUID。
- 没有运行 ESP-IDF 构建、端口识别、烧录、串口、App 或真机操作。

## 仍未验证项

- `main/platform/ble_hid.cpp` 的状态编码重用尚未经过本轮 ESP-IDF 5.5.5 交叉编译；本轮只有共享编码执行测试与平台源码合同。
- 尚未通过真实 BLE GATT read 观察最终状态，也未验证 EasyInput App 0.1.26 对能力字段的实际识别。
- 未烧录、未读串口、未验证 USB／BLE Host Action 真机发送；T06 的传输未验证项保持不变。
