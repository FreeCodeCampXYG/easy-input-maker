# T03 · Host Action v1 纯逻辑实现

- **背景**：T02 已冻结协议；当前 starter 尚未实现 Host Action。
- **目标**：先用宿主测试固定规范小写 UUID 校验、完整配置值解析、Keymap 按下／松开语义和共享报告编码，再做最小纯逻辑实现。
- **输入**：`flow/tasks/T02-Host-Action-v1-固定兼容协议.md`、现有配置／Keymap／App Command 容器。
- **产出路径**：由学员根据现有分层选择 `components/keyboard/` 与 `host_test/` 中的最小文件范围。
- **验收标准**：合法 UUID 接受；大写、长度、连字符或字符错误 fail closed；不增加 version／nil 限制；按下一个事件、松开无事件；固定 wire 字段通过测试；本节点不接 USB／BLE。
- **状态**：已完成纯逻辑实现；USB／BLE 适配、能力声明和烧录仍未开始。

## 实际修改文件

- `components/keyboard/include/keyboard/host_action_protocol.h`
- `components/keyboard/src/host_action_protocol.cpp`
- `components/keyboard/include/keyboard/keymap.h`
- `components/keyboard/src/keymap.cpp`
- `components/keyboard/src/config_payload.cpp`
- `components/keyboard/CMakeLists.txt`
- `host_test/host_action_tests.cpp`
- `host_test/CMakeLists.txt`
- `host_test/firmware_source_contract_tests.cpp`（将 starter 禁止实现断言收窄为平台适配禁止）

## 失败到通过证据

- 先增加测试目标后，`cmake --build build-host --target host_action_tests --parallel` 按预期失败：共享实现文件尚不存在。
- 补齐纯逻辑实现后，定向命令 `ctest --test-dir build-host -R '^host_action_tests$' --output-on-failure` 通过。
- 修正旧的 starter 全局禁止断言后，完整命令 `cmake --build build-host --parallel` 与 `ctest --test-dir build-host --output-on-failure` 通过：58/58。

## 合同覆盖与边界自检

- `host_action:<canonical-lowercase-uuid>` 只接受固定前缀、36 字符小写十六进制 UUID 和固定连字符；非法输入返回 `UnknownAction`，不做大小写转换，不增加 version 或 nil 限制。
- `event_for_action()` 对 Host Action 按下生成一次 `AppCommand`，松开生成 `None`。
- `encode_host_action_app_command()` 固定生成 payload `[0..3] = 0x05/0/1/36`，数据 `[4..39]` 去掉前缀复制 UUID，余量清零；静态测试证明 kind `0x05` 不等于状态 kind `0x04`。
- 未修改 `main/`、`main/platform/usb_hid.cpp`、`main/platform/ble_hid.cpp`；本节点没有 USB/BLE 发送或双发行为证据。

## 未验证项

- 真实 NVS 持久化、USB/BLE 适配与单通道运行时行为尚未验证，留给 T06。
- `"host_action_v1": true` 与 BLE 状态不超过 512 字节留给 T08。
- ESP-IDF 固件构建和实板烧录/观察未在本节点执行。
