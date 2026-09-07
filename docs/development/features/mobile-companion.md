# Mobile Companion v1

这是共享底座的手机旁路协议，不是键盘、钢琴、鼓机的第四槽位。它只观察或在明确授权后临时拥有实体输入；不改变功能槽位、GPIO、音频或 PC HID 默认行为。

读代码：`mobile_companion_protocol.*` 定义版本、lease、connection generation 和 wire；`main/platform/ble_hid.*` 提供独立 GATT；`main/app_main.cpp` 在去抖输入后镜像，在主任务批准 Exclusive。

回归：`mobile_companion_protocol_tests`、`ble_persistence_policy_tests`、`held_keyboard_state_tests`、`transport_routing_tests` 和完整宿主 CTest。实板需分别验证未安装 App、未授权、Mirror、过期、断线、同 handle 重连和 Exclusive 拒绝。
