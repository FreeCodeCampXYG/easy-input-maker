# L1 keyboard：原版键盘（当前槽位 1）

入口：[总览](../README.md) · [共享合同](../shared.md)。稳定文档 ID 为 `keyboard`；不是新的固件枚举。

## 功能与边界

KEY1 语音输入、KEY3 语音编辑属于默认 Keymap；其他键有回车、退格、全选、复制、粘贴、撤销。实际动作由设备配置覆盖，不能仅改 DefaultKeymap 就断言旧设备生效。Host Action、固定文本和 USB/BLE 属于这条原版链。

T3 误输入尚未复现。不要猜测是粘贴绑定，更不能通过关闭原版快捷键“修复”。

## 读代码

1. `main/app_main.cpp`：handle_input_event → event_for_action → dispatch_firmware_event。
2. `components/keyboard/src/keymap.cpp`、`config_state.cpp`、`config_payload.cpp`：动作与平台默认值。
3. `held_keyboard_state.cpp`、`keyboard_snapshot_delivery.cpp`、`transport_routing.cpp`：组合快照、松键、连接生命周期。
4. `main/platform/usb_hid.cpp` / `ble_hid.cpp`：真正向电脑发报告；语音采集另走 keyboard_audio。

## 怎样改

改默认键位先核对保存的配置与兼容策略；改按住行为先加按下/松开和重复输入测试。碰到传输或 app_main 必须读共享合同，钢琴/鼓机应继续消费自己的输入。

先运行：`ctest --test-dir build-host -R "keymap|held_keyboard|keyboard_snapshot|transport_routing|host_action|config_|firmware_source_contract" --output-on-failure`，再完整宿主回归。

实板：原版八键、按住后切槽、USB/BLE 重连、T3 在无敏感测试文本上的复现。不要把正常枚举等同于快捷键工作正常。
