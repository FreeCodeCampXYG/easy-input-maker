# 决策日志（decisions）

> 这里只追加适合公开、对后续协作确有价值的过程决策。

## 2026-08-30 · 固件烧录产物采用标签化 manifest 合同
- 背景：桌面烧录器不能从任意 GitHub 二进制或本机构建目录推断写入偏移和可信来源。
- 决定：仅未来 `firmware-v*` 标签触发的固件 Release 生成 `firmware-manifest.json`、`SHA256SUMS.txt` 和 GitHub 构建溯源；清单固定声明 EasyInput V2.0、ESP32-S3、bootloader/分区表/应用三段及各自偏移和 SHA-256。
- 边界：不把此流程解释为 OTA、签名或自动烧录；标签未推送前不产生 Release，桌面端忽略无清单的历史 Release。

## 2026-08-30 · T10 烧录前审计拒绝申请烧录
- 结论：`SAFE_TO_REQUEST_FLASH = NO`。
- 原因：`host_test/CMakeLists.txt` 将 `host_action_protocol.cpp` 重复加入宿主核心库；虽未阻断 60/60 宿主测试或 ESP-IDF 构建，但未达到“每项变化均合理”的烧录前门槛。
- 边界：硬件、分区、身份、协议、隐私日志和生成物范围均无 FAIL；不在审计节点删除、回退或修复代码，待最小去重修复后重审。

## 2026-08-30 · T06 回归修正：分离 Host Action 与旧 AppCommand 事件
- 发现：T06 初版把所有 `FirmwareEventKind::AppCommand` 都送入 Host Action UUID 编码，可能使 `history`、`settings`、profile 等旧 AppCommand 失败。
- 修正：新增独立 `FirmwareEventKind::HostAction`；只有该事件进入 kind `0x05` 编码，旧 AppCommand 事件类型和平台分支保持原语义。
- 证据：定向 Host Action/源码合同 4 项与完整宿主测试 60/60 通过；协议字段、USB/BLE 发送路径和能力声明未改变。
- 后续验证：复用 ESP-IDF v5.5.5 / esp32s3 增量构建成功，应用镜像为 `0x190810`，App 分区余量 48%。

## 2026-08-30 · Host Action v1 合同在当前实践分支复核
- 背景：用户要求按既有冻结合同推进记录；当前 `my-host-action-practice` 是 Host Action starter，缺少 kind `0x05` 属于课程待实现内容。
- 决定：以 `flow/tasks/T02-Host-Action-v1-固定兼容协议.md` 为权威合同，本节点只在 `flow/` 增加复核记录；保留完整前缀 UUID、`0x11/0x05/0/1/36` 字节合同、按下单发/松开不发、USB/BLE 单通道和第 08 步能力声明边界。
- 核对结论：当前源码未发现 App Command kind `0x05` 占用；`0x04` 仍为状态响应；63 字节容器可承载 `[4..39]` 的 36 字节数据，`[40..62]` 只是余量；无直接冲突。
- 范围：后续实现必须覆盖配置持久化、Keymap/事件、共享编码、USB/BLE 适配和宿主测试；本节点不改代码、不运行验证、不烧录、不访问其他工程。

## 2026-08-30 · T03 只实现 Host Action 纯逻辑层
- 决定：合同一致后先补宿主测试，再在 `components/keyboard/` 实现 UUID fail-closed 校验、Keymap 动作、按下/松开事件和共享 63 字节 payload 编码；不接 USB/BLE，不提前加入能力声明。
- 证据：新增定向测试先因实现缺失失败，完成实现并收窄旧 starter 平台边界断言后，完整宿主测试 58/58 通过。
- 边界：`main/`、USB/BLE 适配、NVS 运行时持久化、ESP-IDF 构建、能力声明和烧录均留在后续节点；v1 字段未扩展。

## 2026-08-30 · T04 八键配置覆盖复用既有解析循环
- 决定：不复制或重写生产解析；以现有 `parse_config_payload()` 的 `KEY1`—`KEY8` 统一 `parse_binding()` 循环为实现，新增参数化宿主覆盖证明每个实体主按键均保留完整 Host Action 配置值。
- 证据：KEY1—KEY8 逐项 press/Keymap/Pressed/Released 断言通过；复制、粘贴、快捷键、固定文字和旧动作回归继续通过，完整宿主测试 58/58。
- 边界：示例 UUID 仅存在宿主测试；未修改 `main/` 或 USB/BLE，T06/T08 的运行时接入与能力声明不提前进行。

## 2026-08-30 · T06 USB/BLE 复用共享 Host Action 帧
- 决定：平台层接收 `FirmwareEventKind::AppCommand` 后调用共享 `encode_host_action_app_command()`，USB 将完整 63 字节帧送入既有 App Command FIFO，BLE 将同一帧送入既有输入队列；不复制 Host Action 常量或重写协议。
- 路由：继续使用 `dispatch_firmware_event()` 的 USB 优先和 owner/epoch 保护；已选通道失败不跨通道补发，避免双发。
- 证据：宿主路由/源码合同和完整 58/58 回归通过；ESP-IDF 编译、真实 USB/BLE 连接和实板烧录留待后续。

## 2026-08-30 · T06 平台源码合同按真实调用形态校验
- 记录：源码合同测试必须匹配 `dispatch_firmware_event()` 当前真实的 `source` 参数和 `send_firmware_event_for_epoch`/BLE 调用形态；一次过严的字符串断言已修正，未改变生产路由逻辑。

## 2026-08-30 · T08 能力声明与 512 字节预算
- 决定：`host_action_v1:true` 仅追加到通用、full、speaker probe 和 fallback 的 `capabilities` 对象；状态响应继续为 `0x04`，Host Action 发送继续为 `0x05`，不把能力混入 App Command payload。
- 预算：speaker probe 仅将 firmware 内容限制为 16 字节并保留全部 probe 指标；normal/compact/battery 溢出时先省略四个可选 cycle 字段，current power 保留。
- 证据：宿主极值最终 BLE wire 最大为 502/512 字节，完整宿主回归 58/58；ESP-IDF/真机状态发布待后续。

## 2026-08-30 · T09 软件侧总验收以实际 CTest 清单为准
- 决定：重新配置后以 `ctest -N` 发现的 60 项为本轮基线，明确注册并执行三个 Host Action 专项目标；不沿用旧的 58 项记录或固定数字。
- 证据：发现 60、执行 60、通过 60、失败 0；ESP-IDF v5.5.5 / esp32s3 默认构建退出 0，生成应用镜像并报告 App 分区剩余 48%。
- 边界：构建/测试证据不等于烧录或实板功能；App、真实 USB/BLE 和设备操作均未执行。

## 2026-08-25 · 完整基线、课程起点与产品专属能力分层
- 背景：完整公共固件已经具备 Host Action v1 和 BLE 配置持久化修复；课程需要让学员按节点实现 Host Action，同时又不能退回到缺少 BLE 修复的旧固件。产品固件另有 OTA、签名和发布流程，不属于本课程目标。
- 决定：Maker `main` 维护完整公共固件基线；`course/host-action-v1-starter` 从同一公共底座中只移除 Host Action 的实现与答案型测试，继续保留 BLE 修复和所有无关公共能力。两者分别用 `course-host-action-v1-complete-v1` 与 `course-host-action-v1-start-v1` 固定课程版本。产品专属 OTA、签名、发布和部署工具不进入 Maker 的任何分支。
- 同步原则：公共固件以行为合同和经过审计的文件差异同步，不做整仓覆盖。仓库专属文档、测试和课程差异可以不同；Maker 的公开隐私规则属于共同安全基线，不能为了机械一致恢复敏感日志。
- 原因：这样既让学员面对真实、最新的 USB／BLE 底座，又只实现一件明确的课程目标；同时把 OTA 的分区迁移、可信发布和恢复风险隔离为独立产品能力。

## 2026-08-22 · Host Action v1 能力只属于状态 JSON
- 背景：Host Action v1 的发送协议已经冻结并接入；第 08 步需要让 App 能从配置状态判断固件能力，同时维持 BLE 状态 512 字节硬上限。
- 决定：`"host_action_v1": true` 只作为各配置状态 `capabilities` 对象中的唯一布尔字段，由完整、紧凑、speaker probe、确认、battery 和共享 fallback 路径声明；不进入 Report ID `0x11` 的 Host Action payload。speaker probe 只把专用 firmware 内容预算收紧为 16 字节并保留全部指标；含 recent power-cycle 的状态超限时，先保留 current power 并只省略四个 cycle 字段。BLE 最终附加编码必须在完整 UTF-8 JSON 不超过 512 字节时才生效。
- 原因：能力发现与动作发送属于两个不同职责；共用状态构造和最终 BLE 编码可避免各发布路径漂移，同时让 0x04 状态响应、0x05 Host Action 和 512 字节上限继续各守原边界。

## 2026-08-22 · 冻结 Host Action v1 兼容协议
- 背景：已经准备好的 EasyInput App 0.1.26 按既有合同向固件同步带真实 UUID 的按键配置；当前 Maker 公开基线尚未实现 Host Action，也没有 App Command kind `0x05`。缺少实现是后续节点要补的能力，不是合同冲突。
- 决定：配置层保存完整的 `host_action:<canonical-lowercase-uuid>`；运行传输层固定使用 Report ID `0x11`、kind `0x05`、chunk index `0`、total chunks `1`、data length `36`，数据区只放去掉 `host_action:` 前缀后的 36 字节 UUID ASCII。63 字节是既有 App Command 消息容器，4 字节头与 36 字节数据占用 payload `[0..39]`，`[40..62]` 仅为容器余量；kind `0x04` 继续专用于状态响应。
- 约束：UUID 只接受规范小写形式；大写、长度错误、连字符位置错误或非法字符必须直接拒绝并 fail closed，不自动转小写，也不增加 UUID version 或 nil UUID 限制。按下发送一次、松开不发送；USB 与 BLE 内容相同并沿用现有单通道选择，不得双发。固件不保存应用路径、名称或 Bundle ID；真实“UUID → 本机应用”映射只存在 EasyInput App 本地，固定示例 UUID 只可用于宿主测试。
- 演进：完整实现必须覆盖配置解析与持久化、Keymap 与事件、共享协议编码、USB／BLE 适配和宿主测试；`"host_action_v1": true` 能力声明及 BLE 状态不超过 512 字节的检查留到第 08 步。v1 字段已冻结，未来不兼容变化必须使用新版本与新能力声明，不能静默改写 v1。
- 原因：现有 App Command kind `0x01`—`0x04`、Report ID `0x11`、63 字节容器及 USB 优先的单通道路由能够容纳这份固定合同，没有实际编号、容量或项目边界冲突。

## 2026-08-22 · README 公众号二维码保留来源仓库独立许可
- 背景：Maker README 缺少 `project-flow-cy` 已公开的公众号二维码，且两个仓库的根许可证不同。
- 决定：从 `project-flow-cy` 提交 `7d3ad181f65e034b7b45cff916f15cfd8fc7db74` 原样复制二维码到 `assets/readme/wechat-qr.jpg`，并保留 GPL-3.0-or-later 许可证文本、固定来源和文件 SHA-256；Maker 其他材料的许可证保持不变。
- 原因：避免外部视觉资产被 Maker 根许可证静默覆盖，让公开分发时的来源、版权和再分发要求可追溯。

## 2026-08-22 · 编码器文字选择使用原生键盘语义
- 背景：EasyInput App 已使用 `cursor + text_caret_select` 配置编码器文字选择；旧 Maker 固件缺少该动作，导致整份配置以 `unknown_action` 被拒绝。
- 决定：规范动作名固定为 `text_caret_select`，旧名称 `mouse_drag_select` 解析为同一动作；该动作只允许位于编码器按压位并配合 `cursor` 模式。短按切换选择状态，旋转发送完整的标准键盘 `Shift+ArrowRight/Left`，3 秒长按继续进入既有系统配置。默认仍为 `scroll_axis_toggle`。
- 原因：复用标准键盘报告可以兼容现有 USB/BLE 输入通道和真实按键状态，不需要新增鼠标拖拽、App Command 或宿主权限协议；严格槽位和模式校验可以继续保持配置 fail-closed。

<!-- 新记录模板：
## YYYY-MM-DD · <决策标题>
- 背景：
- 决定：
- 原因：
-->
