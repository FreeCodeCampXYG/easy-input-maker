# EasyInput Maker 本次优化核对与修改留痕

记录编号：EIM-20260906-01 · 日期：2026-09-06 · 状态：本地候选，未提交/推送/烧录。

本文汇总同一次对话内的缺陷修复、代码审核、兼容优化和文档分层，供维护者转交与排查。路径均相对仓库根目录，不包含私人记忆。文件 SHA-256 见同目录 `snapshot-manifest.json`；源码差异见 `code-changes.patch`。

## 1. 对照版本

- 修改前 Git 基线：`eb833458bed5ea96c49356816197a8cfd4efd4fe`。
- 当前工作分支：`my-host-action-practice`。
- 修改后：没有新 commit；以本记录编号和清单中的文件哈希标识本次快照。分支名和文档日期不能代替具体代码版本。
- 硬件/构建基线：EasyInput V2.0 / ESP32-S3，ESP-IDF 5.5.5。
- 三功能是同一套固件的三个槽位，不是三个独立工程。音乐槽要求启用扬声器相关构建宏。
- 对话开始时没有已跟踪源码改动，存在的 `.codegraph/` 与旧会话归档未纳入补丁或转交包。补丁覆盖本次源码与测试变更，文档/格式配置以完整文件另附。

## 2. 功能修改账本

| 编号 | 所属功能 | 修改前问题 | 本次修改后行为 | 文件/定位符 |
| --- | --- | --- | --- | --- |
| M01 | rhythm 鼓机 | 跨 PCM 帧时丢掉时间余量，非整帧 BPM 长期偏慢 | 保留余量并推进拍位；仍不是采样级渲染 | `components/keyboard/src/drum_sequencer.cpp`：advance |
| M02 | piano + rhythm | 鼓声默认键号 0 被钢琴识别成同一音符 | note_on/note_off 匹配排除 percussion，避免琴键误覆盖鼓声 | `components/keyboard/src/music_synth.cpp`：note_on/note_off |
| M03 | 共享曲目 | 内部曲目加载缺少数组容量和事件校验 | 校验 128 事件上限、音阶/时值/力度/八度；load 拒绝时保留原播放器状态 | `components/keyboard/src/music_sequence_player.cpp`：load |
| M04 | 共享曲目 | 暂停只停时间，当前音符可能持续发声 | 音频 owner 释放当前音符，恢复重触发并保持剩余时值 | 上述播放器及 `include/keyboard/music_sequence_player.h`：advance |
| M05 | click 节拍器 | 忽略 beat_unit，6/8 间隔仍按整拍 | 四分音符 BPM 口径下，6/8 每次 click 为半拍 | `components/keyboard/src/music_synth.cpp`：render |
| M06 | rhythm K8 | 显式曲目请求已取走，重置合成器时无法识别临时播放 | 改用 worker 已加载曲目的 playing 状态临时允许发声，不改保存设置 | `main/platform/speaker_output.cpp`：consume_music_commands |
| M07 | rhythm 旋钮 | 调速失败返回 false，原版滚动/光标接着处理同一旋转 | 音乐槽拥有输入，即使空闲/队列满也不回落到电脑动作 | `main/app_main.cpp`：dispatch_music_volume |
| M08 | MUS1 配置 | 未知调式编号被接受并按大调兜底 | 拒绝未知枚举；合法旧调式保持 | `components/keyboard/src/music_synth.cpp`：validate_music_config |
| M09 | SEQ1 保存回执 | 把播放 applied 当成 saved；控制命令或未落盘也报保存成功 | saved 仅取实际 NVS 结果；扬声器初始化失败也回 ACK | `main/app_main.cpp`：apply_pending_music_sequence |
| M10 | 音频性能 | 每样本重复算包络步长和除法 | apply_config 时缓存两个 uint32 参数，构造复用配置入口 | `music_synth.cpp/.h`：apply_config/advance_voice |
| M11 | 代码结构 | 七类音乐命令重复容量/队列临界区逻辑，另有未读 drum 字段和无用计数 | 统一私有 enqueue_music_command；保留容量、FIFO、去重和松键补偿，删除重复字段/计数 | `main/platform/speaker_output.cpp/.h`；`music_synth.cpp` |

M03 的“保留旧曲目”是播放器 load 接口合同；平台层遇到非法曲目原有停止策略未改。M09 是回执事实修正，电脑端需要确认对 saved=false 的显示和重试行为。

## 3. 文档与规范改动

| 编号 | 内容 | 交付位置 |
| --- | --- | --- |
| D01 | 人工/AI 分层阅读，稳定文档 ID keyboard/piano/rhythm，共享消费者反查 | `docs/development/README.md`、`features/*.md`、`shared.md` |
| D02 | 当前结构图，明确三个槽位仍共用 app_main/音频底座 | `docs/development/feature-map.svg`、`feature-map.png` |
| D03 | 新增功能的输入、进入退出、失败、配置兼容、资源预算与测试模板 | `docs/development/change-template.md` |
| D04 | 根入口与公开进展接入 | `AGENTS.md`、`AI_DEVELOPMENT.md`、`docs/README.md`、`flow/进展.md`、`DEV_STATE.md` |
| D05 | 两空格、100 列目标、不排序 include 的单文件格式配置 | `.clang-format`，已用 clang-format 19.1.2 解析；没有全仓格式化 |
| D06 | 私人长期记忆 L0/L1/L2/L3 分层，旧长文保留为历史 | 已写回用户知识库，不进入公开包；公开指南自包含 |

未来单一功能状态、功能注册表、独立处理器和多功能选择界面是设计建议，本次没有实现。格式化配置解析成功不表示历史所有文件已符合格式。

## 4. 你可以逐项核对

以下复核需要使用包含本次修改的固件；本次尚未生成完整新镜像或烧录。不要用旧设备版本的结果判定新代码已生效。

- [ ] 原版八键、T3、USB/BLE、旋钮原动作仍正常；切槽时没有多余电脑输入（M07）。
- [ ] 钢琴交叠/和弦/松键/尾音正常，鼓声与琴键组合没有误覆盖（M02、M10、M11）。
- [ ] 137 BPM 长期节奏没有持续变慢，6/8 click 节拍符合半拍间隔（M01、M05）。
- [ ] 显式播放曲目后暂停可衰减到无声，恢复保持余下时值；K6 的曲目停止语义仍保留（M04）。
- [ ] 关闭音乐设置后显式 K8 播放，按当前设计仍能发声（M06）。
- [ ] 队列压力下松键不挂音，鼓机旋钮不触发电脑滚动（M07、M11）。
- [ ] 非法曲目/调式被拒绝，保存失败不显示已保存（M03、M08、M09）。
- [ ] 录音抢占、提示音、退出音乐槽和重连后的恢复不回归（共享边界）。

## 5. 已有验证与未完成项

| 证据 | 本次结果 | 限制 |
| --- | --- | --- |
| 修复前失败测试 | M01—M05、M08 有纯逻辑失败证据；M07/M09 有源码合同失败证据 | 源码合同不是设备运行验证 |
| 宿主构建与 CTest | 完整 66/66 通过，包含新增断言 | 不运行真实 I2S/GPIO/NVS |
| PCM 等价 | 8 场景共 768000 样本的优化前后指纹一致 | 仅覆盖测试场景，不是听感或电流测量 |
| 生产函数替身 | 单线程 FIFO/环绕/容量/去重/松键/槽位失败路径通过 | 临时工具未作为稳定测试交付，不证明跨任务竞争 |
| ESP32-S3 目标语法 | app_main、speaker_output、music_synth、music_sequence_player、drum_sequencer 五源文件通过 | 使用已有 5.5.5 编译数据库，未完整链接/构建新固件 |
| 文档/图形/格式 | 链接、UTF-8、SVG 检查、PNG 目视、clang-format 配置解析通过 | 不自动保证未来更新一致 |

未完成：T3 误输入根因；暂停仍维持 I2S 会话的实际功耗；长时堆/栈与碎片化；曲目帧内采样精度和快速命令顺序；已存曲目的恢复播放能力；96 事件内置曲目的持久化方案。未更改 GPIO8、电源生命周期、外部报告编号/容量、默认键位或第三方依赖。

8 声部扩容、音乐旋钮的既有音量规则、K6 停止 K8、96 事件内置旋律是对话之前已有能力，不记成本次新增。

## 6. 出现问题后怎样交给别人或 AI

一起提供本文件、`code-changes.patch`、`snapshot-manifest.json`；最好转发完整核对包。包是局部源码/文档快照，不是完整可构建仓库，也不是固件烧录包，需要接收方已有对应基线。

```text
核对记录：EIM-20260906-01
怀疑关联编号：M__ / D__（不知道可以空）
当前源码 commit / 固件版本 / 构建来源：
功能槽位及实际配置（去掉凭据）：
连接方式：USB / BLE
操作顺序：进入槽位 → 按下/松开 → 是否旋转/切槽/录音
期望行为：
实际行为、频率、是否能稳定复现：
是否只在本次版本出现：
已经运行的测试与未验证项：
```

接手方先核对基线、文件哈希及现有未提交改动，再按编号找函数和测试，复现后小范围修正。不要整笔撤销全部优化；尤其不能为了撤销声音改动连同输入隔离、松键补偿一起删除。

## 7. 留痕与后续更新规则

- 本目录 v1 快照冻结：后续修正新建 v2 记录，写“问题编号 → 触发条件 → 根因 → 修改 → 验证 → 剩余风险”，保留旧包。
- 新 commit、CI run、固件哈希、实板结果产生后，另补交付记录，不把旧的“未验证”历史改写成当时已验证。
- 补丁只供审查；本次 `git apply --reverse --check` 可证明它对应当前代码形态，但不要在已变化工作区直接反向应用。
- 文件哈希用于辨别内容变化，不是数字签名或作者身份证明。换行转换也会改变文件 SHA-256。
- 后续维护经验进入功能 L1/共享 L2；跨项目可复用的经验进入知识库复用知识。禁止把原始对话或临时推理当成经验存档。

详细审计：[上一层审计报告](../input-music-audit-2026-09-06.md)。人工维护：[功能指南](../../development/README.md)。本轮无 Git 提交、标签或发布记录。
