# T06 · Host Action v1 Vendor HID 发送接入

- **背景**：T03 的共享编码需要接入现有 App Command 发送路径，不能在 USB 与 BLE 各写一套协议。
- **目标**：USB／BLE 复用同一编码结果，并沿用现有单通道选择和连接 owner／epoch 保护。
- **输入**：T02 固定 wire 合同、T03 共享编码、现有 Vendor HID App Command 路径。
- **产出路径**：必要的最小平台适配与宿主源码合同测试。
- **验收标准**：Report ID、kind、单 chunk 和 36 字节数据不变；状态 kind 继续为 `0x04`；USB 优先否则 BLE且不双发；不改 HID 描述符、BLE GATT 或设备身份。
- **状态**：待办
