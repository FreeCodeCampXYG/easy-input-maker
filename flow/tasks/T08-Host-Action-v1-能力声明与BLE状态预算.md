# T08 · Host Action v1 能力声明与 BLE 状态预算

- **背景**：App 需要从状态 JSON 发现能力，而 BLE 最终发布状态有 512 字节硬上限。
- **目标**：把唯一布尔能力值加入实际状态构造路径，并验证每个发布变体的最终 UTF-8 字节数。
- **输入**：T02 合同、T06 发送接入、现有完整／紧凑／诊断／battery／fallback 状态路径。
- **产出路径**：最小状态实现与宿主预算测试。
- **验收标准**：能力是布尔 `true`、只在 capabilities 中出现一次；`0x04`／`0x05` 职责不变；所有实际发布变体不超过 512 字节；不得提高上限或静默删除无关状态字段。
- **状态**：已完成；ESP-IDF 与实板验证仍待后续。

## 实际修改文件

- `components/keyboard/include/keyboard/config_status.h`
- `components/keyboard/src/config_status.cpp`
- `host_test/config_status_tests.cpp`
- `host_test/firmware_source_contract_tests.cpp`
- `flow/` 中本任务卡、计划、决策与进展记录

## 能力声明路径

- 通用 `append_sync_core()`：覆盖 compact、battery、确认和其各回退分支。
- `build_full_status_json()`：覆盖完整状态。
- `build_speaker_probe_status_json()`：覆盖 speaker probe，不删除其现有指标。
- `kConfigStatusFallbackJson`：覆盖 BLE 发布进入最终 fallback 的路径。
- 每条路径仅含一个 JSON 布尔 `"host_action_v1":true`；不写顶层字段、不写字符串或 false，不改其他能力字段。

## 预算与回退证据

- speaker probe 固件内容预算固定为 16 字节；全部既有 speaker 指标、字段名和 JSON 结构保留，最坏样本的最终 BLE wire 为 502 字节。
- normal/compact 与 battery 路径保留 current power 的 `mode`、`inactive_ms`、`deep_sleep_block`、`last_wake`；周期字段能放下时保留，超限时先只省略 `cycle_seq`、`cycle_inactive_ms`、`cycle_deep_sleep`、`cycle_wake`。
- 最终 BLE wire 字节数（宿主极值样本）：full 378（余 134）、compact-cycle 465（余 47）、speaker-probe 502（余 10）、confirmation 420（余 92）、battery 461（余 51）、fallback 241（余 271）；均不超过 512。

## 验证

- 定向：`ctest --test-dir build-host -R '^(config_status_tests|firmware_source_contract_tests)$' --output-on-failure`，2/2 通过。
- 完整：`cmake --build build-host --parallel` + `ctest --test-dir build-host --output-on-failure`，58/58 通过。
- 未修改 Host Action `0x05` 发送、状态响应 `0x04`、63 字节 App Command 容器、HID 描述符、BLE GATT、设备身份、GPIO、BOOT、电源或 Flash 分区。

## 未验证项

- 未执行 ESP-IDF 构建、App 联调、真实 BLE 发布/读取或实板烧录。
