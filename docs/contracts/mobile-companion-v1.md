# Mobile Companion v1

`Mobile Companion v1` 是 EasyInputApp 的独立 BLE GATT 旁路协议，不是 HID Report、`FirmwareEventKind::AppCommand`，也不是 `FirmwareEventKind::HostAction`。

## 服务与协商

- Service UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320010`
- Control UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320011`，Write / Write Without Response。
- Capability UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320012`，Read。
- ACK UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320013`，Notify。
- Event UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320014`，Notify。

所有帧固定 16 字节、小端：`0xE1 | version | type | command | request_id:u16 | payload_length:u8 | flags:u8 | generation:u32 | payload[2] | crc16:u16`。`payload[0]` 为能力，`payload[1]` 为 lease 秒数；CRC16-IBM 覆盖偏移 0..13。Control 支持申请、续租、释放、能力查询及配置读写；ACK 返回结果和能力。未知版本、长度、枚举值或 CRC 必须拒绝。

## 会话与安全边界

- Control 只接受已加密且已绑定的 BLE 连接；配对不是 Exclusive 的隐式授权。
- 每一个 GAP CONNECT 都分配 Mobile 独立 generation；HID owner generation 不复用。handle 重用、断线、超时或 generation 不匹配均清除会话，不重放事件。
- lease 默认 15 秒，最大 30 秒。续租必须来自同一 connection + generation；其他连接不能覆盖、续租或释放活跃会话。
- `Mirror` 只复制已去抖的实体 KEY / encoder 事件，PC USB/BLE HID 路由与 held keyboard state 不变。
- 当前 v1 固件只授予 `Mirror`；`Exclusive` 枚举保留但请求被拒绝，避免未完成的输入归属切换影响 PC HID。拒绝不会清空 PC 状态；断线或过期回退到原 PC 路由。

## 兼容性与证据

HID Report Map、Usage Page、`0x10` 配置 Feature 与 `0x11` legacy AppCommand 保持原合同。新服务需要两个额外 CCCD，产品预算从每 peer 8 调整为 10，NimBLE 容量从 32 调整为 40。宿主协议、会话代际、CCCD 容量和 HID 路由测试已通过；目标固件 CI、EasyInputApp 联调和实板断线/重连测试待验证。
