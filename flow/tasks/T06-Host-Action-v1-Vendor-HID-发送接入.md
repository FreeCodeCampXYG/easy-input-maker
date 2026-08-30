# T06 · Host Action v1 Vendor HID 发送接入

- **背景**：T03 的共享编码需要接入现有 App Command 发送路径，不能在 USB 与 BLE 各写一套协议。
- **目标**：USB／BLE 复用同一编码结果，并沿用现有单通道选择和连接 owner／epoch 保护。
- **输入**：T02 固定 wire 合同、T03 共享编码、现有 Vendor HID App Command 路径。
- **产出路径**：必要的最小平台适配与宿主源码合同测试。
- **验收标准**：Report ID、kind、单 chunk 和 36 字节数据不变；状态 kind 继续为 `0x04`；USB 优先否则 BLE且不双发；不改 HID 描述符、BLE GATT 或设备身份。
- **状态**：已完成；平台运行时真机行为仍待验证。

## 实际修改文件

- `main/platform/usb_hid.h`、`main/platform/usb_hid.cpp`
- `main/platform/ble_hid.h`、`main/platform/ble_hid.cpp`
- `host_test/firmware_source_contract_tests.cpp`
- `host_test/host_action_tests.cpp`
- `flow/` 中本任务记录与进展/决策记录

## 接入路径与合同证据

- `main/app_main.cpp::dispatch_firmware_event()` 对 `FirmwareEventKind::AppCommand` 继续调用 `route_for_firmware_event()`，USB 优先；USB epoch 失败后直接返回，不补发 BLE。
- USB：`UsbHidTransport::send_firmware_event_for_epoch()` → `encode_host_action_app_command()` → `queue_app_command_payload_for_epoch()` → 既有 `pending_app_command_reports_` → `try_send_app_command_report()` → `tud_hid_report()`。
- BLE：`BleHidTransport::send_firmware_event_for_owner()` → `send_host_action_app_command()` → `encode_host_action_app_command()` → `send_input_report()` → 既有 BLE 调度器/owner 发送路径。
- 两侧均复用共享编码结果和 `kHostActionPayloadLen == kAppCommandReportPayloadLen` 静态断言；未复制 Host Action kind 常量。
- 共享编码固定 `0x05/0/1/36`，UUID 去前缀写入 `[4..39]`，其余 `[40..62]` 保持零；状态 kind `0x04` 未改动。

## 单通道与回归

- 宿主路由测试证明 AppCommand 仍为 `UsbFirst`；源码合同测试证明 `dispatch_firmware_event()` 的 USB/BLE 分支和两侧共享编码入口存在。
- 定向 `ctest --test-dir build-host -R '^host_action_tests$' --output-on-failure`：通过。
- 完整 `cmake --build build-host --parallel` + `ctest --test-dir build-host --output-on-failure`：58/58 通过；原有 App Command、键盘 HID、复制/粘贴、快捷键和固定文字测试继续通过。
- 中间失败：平台源码合同断言初次使用了不存在的 `source_input` 调用文本；按真实 `app->ble.send_firmware_event(source, event)` 修正后完整回归通过。

## 未验证项

- 当前环境未执行 ESP-IDF 固件构建；USB/BLE 真实设备发送、连接断开和 owner/epoch 运行时行为仍待 T09/实板验证。
- 未加入 `host_action_v1` 能力声明，未修改 HID 描述符、BLE GATT、设备身份、GPIO、BOOT、电源或 Flash 分区。
