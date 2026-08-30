# T04 · Host Action v1 八键配置覆盖

- **背景**：八个实体主按键应复用同一配置解析链路，不能复制八套实现。
- **目标**：补参数化宿主测试，逐项证明 KEY1—KEY8 都能保存完整配置值，并产生正确的按下／松开事件。
- **输入**：T02 固定合同、T03 纯逻辑实现、现有八键配置循环。
- **产出路径**：`host_test/` 中的最小覆盖测试；只有测试证明存在真实缺口时才修改生产逻辑。
- **验收标准**：八键逐项通过；每次 press 恰好一个事件，release 无事件；复制、粘贴、快捷键和固定文字回归；示例 UUID 只存在宿主测试。
- **状态**：已完成

## 实际修改文件

- `host_test/host_action_tests.cpp`：增加 KEY1—KEY8 参数化配置覆盖。
- 未修改生产解析逻辑：现有 `parse_config_payload()` 已通过同一 `parse_binding()` 循环处理 8 个实体主按键。

## 八键逐项证据

- KEY1、KEY2、KEY3、KEY4、KEY5、KEY6、KEY7、KEY8 均使用同一合法示例 UUID 的 `press` 配置解析成功。
- 每个按键进入 Keymap 后均为 `ActionKind::HostAction`，`Action.hotkey` 保留完整 `host_action:` 前缀。
- 每个按键的 `Pressed` 仅生成一次 `FirmwareEventKind::AppCommand`；对应 `Released` 生成 `FirmwareEventKind::None`，不产生第二个 Host Action。
- 示例 UUID 只出现在 `host_test/host_action_tests.cpp`；默认 Keymap、生产配置和 `main/` 未写入该值。

## 兼容与回归证据

- 现有复制、粘贴、快捷键、固定文字、旧动作和配置迁移测试均通过。
- 定向：`ctest --test-dir build-host -R '^host_action_tests$' --output-on-failure`，1/1 通过。
- 完整：`cmake --build build-host --parallel` 与 `ctest --test-dir build-host --output-on-failure`，58/58 通过。
- `git diff --name-status -- main` 无输出；`main/platform/`、USB/BLE 适配、能力声明、GPIO、BOOT、电源、分区和设备身份未修改。

## 未验证项

- USB/BLE 运行时发送与单通道行为留给 T06；能力声明及 BLE 512 字节预算留给 T08。
- ESP-IDF 固件构建、真实 NVS 设备持久化和实板烧录/观察未在本节点执行。
