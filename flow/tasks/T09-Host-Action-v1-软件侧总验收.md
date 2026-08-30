# T09 · Host Action v1 软件侧总验收

- **背景**：局部测试通过不等于完整固件可构建。
- **目标**：重新配置、发现并运行全部宿主测试，再用项目锁定的 ESP-IDF 5.5.5 完成 `v2`／`esp32s3` 默认构建。
- **输入**：T03、T04、T06、T08 的当前工作区和测试清单。
- **产出路径**：被忽略的 `build-host/`、`build-*/` 以及公开的脱敏验证记录。
- **验收标准**：发现数等于执行数等于通过数，0 失败；默认构建成功并记录镜像与分区余量；清楚区分逻辑通过、构建通过和尚未烧录／真机验证。
- **状态**：已完成软件侧验收；App、实板和烧录仍未验证。

## 本轮实际命令与结果

1. `cmake -S host_test -B build-host -DCMAKE_BUILD_TYPE=Debug`：退出 0。
2. `ctest --test-dir build-host -N`：实际发现 60 项；清单中包含 `host_action_protocol_tests`、`host_action_key_bindings_tests`、`host_action_capability_status_tests`。
3. `cmake --build build-host --parallel`：退出 0。
4. `ctest --test-dir build-host --output-on-failure`：执行 60 项、通过 60 项、失败 0 项，退出 0。
5. ESP-IDF 环境复核：`ESP-IDF v5.5.5`，目标 `esp32s3`；未调用 `set-target`。
6. `idf.py build`（通过项目锁定 v5.5.5 环境）：退出 0，`Project build complete`。

## CTest 清单核对

- 基线测试：本轮重新配置后总发现数为 60；相比 T08 记录的 58，新增/明确注册 3 个 Host Action 专项目标，同时移除旧的泛化 `host_action_tests` 注册名，净增 2。
- Host Action 专项：`host_action_protocol_tests`、`host_action_key_bindings_tests`、`host_action_capability_status_tests` 均已注册、构建并执行通过。
- 完整清单共 60 项，实际执行数与发现数一致，60/60 通过，失败数为 0；没有跳过、删除或缩减测试。

## 构建产物与空间

- `build/easy_input_keyboard.bin`：1,640,384 bytes（`0x1907c0`）。
- `build/easy_input_keyboard.elf`：21,302,252 bytes。
- `build/bootloader/bootloader.bin`：20,832 bytes；`build/partition_table/partition-table.bin`：3,072 bytes。
- App 分区：`0x300000`，剩余 `0x16f840`（48%）。
- 刷写元数据：`build/flash_args`、`build/flasher_args.json`。

## Warning 与环境修复

- 首次激活遇到项目构建缓存记录的旧 Python venv 与 EIM 新 venv 不一致；确认旧 venv 存在后使用其执行 `idf.py`，未改源码或目标。
- EIM/官方安装补齐 v5.5.5 工具链和 Python 依赖时，直连 GitHub 超时；临时使用本机代理完成安装。未升级 ESP-IDF。
- 构建提示 `esp_blockdev` 目录缺少 CMakeLists、ESP-TEE 仅支持其他芯片，以及旧 Python venv 警告；均未阻断最终构建。

## 重新编译范围与禁止范围

- 本轮 ESP-IDF 重新编译实际包含 Host Action 相关 `components/keyboard/src/host_action_protocol.cpp`、`config_status.cpp`、`config_payload.cpp`、`keymap.cpp` 以及 `main/platform/usb_hid.cpp`、`ble_hid.cpp`。
- 未修改 Report ID `0x11`、kind `0x04/0x05`、63 字节容器、HID 描述符、BLE GATT、设备身份、GPIO、BOOT、GPIO8 电源、Flash 分区；未识别设备、读串口、烧录或运行 App。

## 证据边界

- 宿主测试通过只证明当前 Windows 宿主代码逻辑与合同测试通过。
- ESP-IDF 构建通过只证明当前源码可为 `esp32s3` 生成固件。
- 烧录、真实 USB/BLE 发送、App 联调、实板功能和用户体验仍未验证。
