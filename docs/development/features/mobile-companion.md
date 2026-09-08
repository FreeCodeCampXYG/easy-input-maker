# Mobile Companion v1

这是共享底座的手机旁路协议，不是键盘、钢琴、鼓机的第四槽位。v1 默认只支持 Mirror；Exclusive 仅保留协议枚举，当前固件拒绝授予。它不改变功能槽位、GPIO、音频或 PC HID 默认行为。

读代码：`mobile_companion_protocol.*` 定义固定帧、CRC、lease、connection generation 和 wire；`mobile_companion_session.*` 定义会话状态机；`mobile_companion_event_cache.*` 定义有限恢复窗口；`main/platform/ble_hid.*` 提供独立 GATT 和 NVS 短 ID；`main/app_main.cpp` 在去抖输入后镜像。

回归：`mobile_companion_protocol_tests`、`ble_persistence_policy_tests`、`held_keyboard_state_tests`、`transport_routing_tests` 和完整宿主 CTest。实板需分别验证未安装 App、未授权、Mirror、过期、断线、同 handle 重连和 Exclusive 拒绝。

配置读写使用独立 Config 分片，最大 2048 字节、每片 2 字节、严格顺序和整段 JSON CRC 校验；完整解析/持久化前保留旧配置。手机绑定默认是 Mirror 会话 Overlay。

当前已完成纯逻辑分片装配和 `MobileBindingOverlay` 校验；真实 GATT ReadConfig/WriteConfig、NVS 原子提交和配置 ACK 仍待平台接入。

Identity 特征只读返回 NVS 持久化的 8 字节随机短 ID；事件缓存固定 32 条并支持有限窗口查询。音乐最终 PCM 增益默认 125% 且饱和裁剪，音量旋钮语义保持 5%—100%。
