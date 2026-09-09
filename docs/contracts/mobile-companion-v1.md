# Mobile Companion v1

`Mobile Companion v1` 是 EasyInputApp 的独立 BLE GATT 旁路协议，不是 HID Report、`FirmwareEventKind::AppCommand`，也不是 `FirmwareEventKind::HostAction`。

## 服务与协商

- Service UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320010`
- Control UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320011`，Write / Write Without Response。
- Capability UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320012`，Read。
- ACK UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320013`，Notify。
- Event UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320014`，Notify。
- Identity UUID：`7d2f4d10-6f6b-4a2d-8b01-6d4653320015`，Read；返回 8 字节大写短 ID，不返回 MAC、Wi-Fi 或真实序列号。旧版 App 不识别该特征时仍可按原五特征降级，但不得把设备名当作身份。

所有帧固定 16 字节、小端：`0xE1 | version | type | command | request_id:u16 | payload_length:u8 | flags:u8 | generation:u32 | payload[2] | crc16:u16`。`payload[0]` 为能力，`payload[1]` 为 lease 秒数；CRC16-IBM 覆盖偏移 0..13。首次申请/能力查询 generation 必须为 0，ACK 返回 session generation；续租/释放必须回传该 session generation。Control 支持申请、续租、释放、能力查询及配置读写；ACK 返回结果和能力。未知版本、长度、枚举值或 CRC 必须拒绝。

## 会话与安全边界

- Control 只接受已加密且已绑定的 BLE 连接；配对不是 Exclusive 的隐式授权。
- 每一个 GAP CONNECT 都分配 Mobile 独立 generation；HID owner generation 不复用。handle 重用、断线、超时或 generation 不匹配均清除会话，不重放事件。
- lease 默认 15 秒，最大 30 秒。续租必须来自同一 connection + generation；其他连接不能覆盖、续租或释放活跃会话。
- `Mirror` 只复制已去抖的实体 KEY1—KEY8 Pressed/Released 事件，PC USB/BLE HID 路由与 held keyboard state 不变；旋钮不进入手机游戏事件。
- 当前 v1 固件只授予 `Mirror`；`Exclusive` 枚举保留但请求被拒绝，避免未完成的输入归属切换影响 PC HID。拒绝不会清空 PC 状态；断线或过期回退到原 PC 路由。

## 配置与按键绑定

- `WriteConfig` / `ReadConfig` 使用 Config frame 分片：`magic/version/type/command/request_id/chunk_index/total_chunks/total_len/payload_crc/data[2]`；配置内容 CRC 覆盖完整 UTF-8 JSON，分片末两字节用于数据。最大配置 2048 字节，禁止截断字符串。
- 分片必须从 0 开始严格按序；重复分片返回 Duplicate，乱序返回 OutOfOrder，CRC/长度错误或 10 秒超时丢弃临时缓冲，当前配置保持不变。只有完整解析和持久化成功才确认写入。
- 手机绑定是 Mirror 会话 Overlay，不改设备全局默认键位。action kind 沿用 `Disabled`、`VoicePttHold`、`EditPttHold`、`Hotkey`、`FixedText`、`OpenHistory`、`Settings`、`HostAction`、`SelectAll`、`Copy`、`Paste`、`Undo`。
- `MobileBindingOverlay` 为每个 `InputId` 保存独立 action/参数；按下/释放沿用既有语义，平台差异只存在于 action 参数。`FixedText` 最大 512 字节，非法 action 或超长文本拒绝。
- 事件仅对 MirrorActive 的 KEY1—KEY8 发送：`event_id:u16` 在 4..5，flags 在 7（bit0/1=按下/释放，bit2..=input_id 与能力 flags），generation 在 8..11，sequence_low16 在 12..13，14..15 为 CRC。KEY1/KEY3/KEY8 的能力 flags 保持既有值，KEY2/KEY4/KEY5/KEY6/KEY7 的能力 flags 为 0。释放、过期、断线或 generation 变化后停止发送。
- `QueryEvents` 可在重连后请求有限恢复窗口：generation 必须匹配当前会话，payload 两字节携带最近确认 sequence 的低 16 位；缓存固定 32 条，溢出丢弃最旧事件并增加计数。旧 generation、窗口溢出、重复或乱序均返回明确结果，不阻塞实体输入。

游戏旁路验收边界：当前固件 MirrorActive 时允许 KEY1—KEY8 Pressed/Released；`publish_mobile_input()` 先使会话过期状态失效，再从 endpoint 表选择唯一有效 Mirror connection。没有有效 endpoint、租约已过期或 generation 不匹配时不发送手机事件，但 `handle_input_event()` 仍继续原 PC HID 路由。手机游戏的方向/动作映射属于 EasyInputApp，不写回 Maker 默认 Keymap。

Device Identity 在首次读取时生成 8 字节随机大写短 ID 并保存 NVS；读取失败不回退到 MAC、Wi-Fi 或芯片真实序列号。旧版 App 不认识 Identity 特征时仍可按原五个 UUID 工作。

样例（末两字节 CRC 需按偏移 0..13 重新计算）：Mirror 申请 `E1 01 01 01 01 00 02 00 00 00 00 00 01 0F CRC16LE`；能力查询 `E1 01 01 04 02 00 02 00 00 00 00 00 00 00 CRC16LE`。

## 兼容性与证据

### 首连失败与广播恢复

辅助 Mobile Companion 连接不等于 HID owner。`control_conn_handle_` 仅在连接仍由 NimBLE `ble_gap_conn_find` 证明存活时才会使已连接 HID 进入控制广播策略；查找失败的 stale handle 必须在主任务中清除，并同时释放该连接的 CONFIG/Mobile endpoint、pending request 与状态快照，再触发广播 reconcile。`BLE_GAP_EVENT_ENC_CHANGE` 返回失败时同样清理辅助连接；HID owner 的失败继续走原 owner recovery。该修复只处理连接生命周期，不改变 PC HID、HostAction、legacy AppCommand 或 Mirror 事件语义。

HID Report Map、Usage Page、`0x10` 配置 Feature 与 `0x11` legacy AppCommand 保持原合同。新服务需要两个额外 CCCD，产品预算从每 peer 8 调整为 10，NimBLE 容量从 32 调整为 40。Mirror 通知提交后 `publish_mobile_input()` 不返回消费结果，调用方继续执行 PC HID 路由；当前固件没有已授权 Exclusive。宿主协议、会话代际、事件缓存、八键旁路与 HID 路由测试已通过；目标固件 CI、EasyInputApp 联调和实板断线/重连测试待验证。

USB 诊断边界：生产固件继续保持 `VID=0x303A/PID=0x1006` HID-only，不增加 CDC 或新的 USB profile。电脑端 Python/Flasher 复用既有 Vendor HID `0x13` 状态请求和 `0x11/0x04` 分片回报读取 BLE 快照；该请求不会改变键盘、鼠标、供应商 HID Report ID 或输入行为。

诊断采集边界：BLE 事件只在固件内保留有限运行态快照，不写入 NVS；主机负责轮询、实时显示和持久化日志。手机 BLE 控制仍由 EasyInputApp 发起，反复测试不需要重新编译或烧录同一固件。
