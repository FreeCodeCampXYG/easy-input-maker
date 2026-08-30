# T13 · Host Action v1 协议核对与记录

> 本节点只记录既有冻结合同与当前 Maker 分支核对结果，不实现 Host Action。

## 操作前文件状态

- 分支：`my-host-action-practice`。
- HEAD：`71c2b4b`（`ci: publish firmware build artifacts`）；标签 `my-host-action-practice-v1.1` 指向该提交。
- `git status --short`：`M DEV_STATE.md`、`?? .codegraph/`、`?? flow/tasks/T12-Host-Action-v1-实践分支只读起点.md`。
- 操作前 `git diff --name-status -- flow`：无输出；既有非 `flow/` 状态不属于本节点，保持不动。

## 当前基线冲突门

- `components/keyboard/src/config_payload.cpp`、`components/keyboard/include/keyboard/keymap.h`、`components/keyboard/src/keymap.cpp` 尚无 Host Action 动作或 `host_action:` 解析。
- `main/platform/usb_hid.cpp` 与 `main/platform/ble_hid.cpp` 现有 App Command kind 为 `0x01`、`0x02`、`0x03`；未发现 App Command kind `0x05` 实现。源码中其他 `0x05` 属于 HID Usage/键码等不同编号空间。
- 现有 App Command 使用 Report ID `0x11`、63 字节 payload、4 字节头；`components/keyboard/include/keyboard/status_hid_protocol.h:20` 将 `0x04` 固定为状态响应 kind。固定字段占 `[0..39]`，余量 `[40..62]` 足够承载，不存在容量冲突。
- `components/keyboard/src/transport_routing.cpp` 与 `main/app_main.cpp` 保留 USB 优先、连接 owner/epoch 和失败不迁移的单通道规则；与合同的禁止双发要求一致。

**冲突门结论：** 当前公开 starter 基线缺少 Host Action/kind `0x05` 是后续节点的待实现缺口，不是冲突；没有发现现有项目明确占用 `0x05`，也没有发现容器无法承载合同字段。因此继续记录，不修改代码。

## 固定合同逐项复述与自检

### 配置、校验与持久化

- [x] 配置层保存完整 `host_action:<canonical-lowercase-uuid>`。
- [x] UUID 必须是 36 个 ASCII 字符的规范小写格式，连字符位于第 9、14、19、24 位，其余只允许 `0-9`/`a-f`。
- [x] 大写、长度错误、连字符位置错误或非法字符直接拒绝并 fail closed；不自动转小写。
- [x] 不增加 UUID version 限制，也不增加 nil UUID 限制。
- [x] Keymap、运行状态和 NVS 持久化保留完整前缀；仅传输编码去掉 `host_action:`。

### 传输字节合同

- [x] Report ID `0x11`；kind `0x05`；chunk index `0`；total chunks `1`；data length `36`。
- [x] 数据区 `[4..39]` 只放不带前缀的 36 字节 UUID ASCII。
- [x] `[40..62]` 只是现有 63 字节 App Command 容器余量，不是 Host Action 数据长度。
- [x] kind `0x04` 继续专用于状态响应，不复用、不改写。
- [x] USB 与 BLE 使用完全相同内容，沿用现有单通道选择，禁止双发。

### 事件与职责边界

- [x] 按下只发送一次 Host Action v1；松开不发送，也不补发释放消息。
- [x] 非规范配置不生成 Host Action 事件，保持 fail closed。
- [x] 固件不保存应用路径、名称或 Bundle ID。
- [x] 真实 UUID 到本机应用的映射只保存在 EasyInput App 本地；同步给固件的配置包含真机 UUID。
- [x] 固定示例 UUID 只允许出现在宿主测试，不作为默认映射或固件身份。

### 版本与完整改动层

- [x] v1 字段已冻结；不兼容变化必须使用新版本和新的能力声明，不能静默改写 v1。
- [ ] 后续实现必须覆盖：配置解析与持久化、Keymap 与事件、共享协议编码、USB/BLE 适配、宿主测试。
- [ ] 第 08 步才加入能力声明 `"host_action_v1": true`，并检查所有相关 BLE 状态不超过 512 字节；本节点不提前声明。

## 不承担范围

- 本节点不修改代码、测试、构建配置、硬件引脚、BOOT、GPIO8、分区、设备身份或通信容器。
- 本节点不运行测试、构建、烧录，不访问设备，不读取或修改其他电脑端工程。
- 本节点不自行设计字段、校验、分片、释放消息、第二种 Host Action 消息或新的 UUID 限制。

## 结果

- 合同记录与当前分支冲突门核对完成；仅新增/修改 `flow/` 项目记录。
- 本节点没有新增代码变更；操作前后的非 `flow/` 工作区状态保持不变。
