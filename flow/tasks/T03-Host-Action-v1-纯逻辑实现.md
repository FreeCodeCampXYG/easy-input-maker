# T03 · Host Action v1 纯逻辑实现

> 本任务严格落实 `flow/tasks/T02-Host-Action-v1-固定兼容协议.md`，不重新设计合同。本节点只实现可宿主测试的纯逻辑，不接入 `main/`、USB 或 BLE。

- **背景**：T02 已冻结配置格式、UUID 失败语义、Report ID `0x11`、kind `0x05`、单块 `0/1`、data length `36`、线上 UUID 无前缀及 `0x04` 状态保留等合同。
- **目标**：先补宿主失败测试，再最小实现配置解析、Keymap／固件事件和共享协议编码。
- **允许范围**：`components/keyboard/`、`host_test/` 与根级 `flow/`。
- **禁止范围**：不修改 `main/`、`main/platform/usb_hid.*`、`main/platform/ble_hid.*` 或其他传输适配；不增加能力声明；不运行 ESP-IDF 构建；不访问或烧录设备。
- **状态**：纯逻辑完成；传输及设备验证未开始。

## 合同复核

- 配置值固定为完整 `host_action:<canonical-lowercase-uuid>`。
- 运行报告固定为 Report ID `0x11`，payload `[0..3]` 为 `0x05 / 0 / 1 / 36`，payload `[4..39]` 只含无前缀 UUID ASCII。
- 大写、错误长度、错误连字符位置或非法字符直接拒绝，不自动转小写；不增加 UUID version 或 nil UUID 限制。
- 按下生成一次 `HostAction`，松开生成 `None`。
- 状态响应 kind `0x04` 不变且不被占用。

结论：实现前合同完整且一致，没有停止条件。

## 操作前状态

- 分支与提交：`main`，`34087cd40d24d23579da0357973ebc1a37e7ce7c`。
- 操作前仅有既有 `flow/` 修改和任务卡；`components/keyboard/`、`host_test/`、`main/` 均无差异。

## 实际修改文件

### 纯逻辑实现

- `components/keyboard/CMakeLists.txt`
- `components/keyboard/include/keyboard/host_action_protocol.h`（新增）
- `components/keyboard/include/keyboard/keymap.h`
- `components/keyboard/src/host_action_protocol.cpp`（新增）
- `components/keyboard/src/config_payload.cpp`
- `components/keyboard/src/keymap.cpp`

### 宿主测试

- `host_test/CMakeLists.txt`
- `host_test/host_action_protocol_tests.cpp`（新增）
- `host_test/config_payload_tests.cpp`
- `host_test/config_state_tests.cpp`
- `host_test/keymap_tests.cpp`

## 实现结果

- 配置解析只把格式完全规范的 `host_action:<UUID>` 转成 `ActionKind::HostAction`，并在 `Action::host_action` 保留完整字符串。
- `ConfigState` 继续沿用候选配置 fail-closed：无效 Host Action 返回 `UnknownAction`，不会覆盖此前有效 Keymap 或 `last_applied_json()`。
- `event_for_action()` 对有效 Host Action 在 `Pressed` 返回一次 `FirmwareEventKind::HostAction`，在 `Released` 返回 `None`；手工构造的无效动作也不会生成事件。
- `encode_host_action_v1()` 再次校验完整配置值，只在成功时写出报告；失败时不修改调用方输出。
- 共享报告固定携带 Report ID `0x11`，payload 头为 `0x05 / 0 / 1 / 36`，数据区只复制 UUID 的 36 个 ASCII 字节。
- 实现不检查 UUID version 或 nil；规范 nil UUID 和全 `f` UUID 的宿主测试均接受。

## 测试命令

当前普通 shell 未直接暴露 `cmake`；实际执行使用 ESP-IDF 工具链已安装的 CMake 3.30.2。为遵守公开项目不得记录本机 SDK 绝对路径的规则，下面记录等价、可移植的项目命令：

```bash
cmake -S host_test -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --target host_action_protocol_tests config_payload_tests config_state_tests keymap_tests -j4
ctest --test-dir build-host --output-on-failure -R '^(host_action_protocol_tests|config_payload_tests|config_state_tests|keymap_tests)$'
cmake --build build-host -j4
ctest --test-dir build-host --output-on-failure
```

## 失败到通过的证据

- **红灯**：测试与测试注册先写入；CMake 配置成功，定向构建在 `host_test/host_action_protocol_tests.cpp:7` 失败，最小错误为 `fatal error: 'keyboard/host_action_protocol.h' file not found`。此时实现文件尚不存在。
- **定向绿灯**：补齐最小纯逻辑实现后，`host_action_protocol_tests`、`config_payload_tests`、`config_state_tests`、`keymap_tests` 共 4/4 通过，0 失败，总用时 1.56 秒。
- **完整绿灯**：全部宿主测试 57/57 通过，0 失败，总用时 16.70 秒。
- `git diff --check` 通过。

## USB／BLE 未修改证据

- `git diff -- main main/platform` 无输出。
- `git diff --name-only -- main main/platform` 无输出。
- `rg 'host_action|HostAction|kHostAction' main main/platform` 无输出。
- 因此本节点只有共享编码结果，没有任何 USB／BLE 发送调用或运行时分发接线。

## 仍未验证与不承担范围

- 未接入 `main/app_main.cpp`，未验证真实输入事件能到达运输层。
- 未接 USB 或 BLE 适配，尚不能证明任一实际传输会发出 Host Action，也没有 USB／BLE 端到端内容一致证据。
- 未修改或专项验证 NVS 持久化；当前只证明解析结果、`ConfigState` 和原始已应用 JSON 保留完整值。
- 未运行 ESP-IDF 5.5.5 默认构建；宿主测试通过不能替代固件构建。
- 未实现第 08 步 `"host_action_v1": true`，也未因该字段重新检查 BLE 状态 512 字节预算。
- 未进行 App 0.1.26 同步、烧录、串口或实板验证。
