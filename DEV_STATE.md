# EasyInput Maker 开发状态

## 2026-09-06：本次对话核对留痕

- 汇总本次 M01—M11 修复/优化与 D01—D06 文档分层，入口 `docs/review/session-2026-09-06/README.md`；以基线 commit、代码补丁和文件 SHA-256 区分本地候选与设备版本。
- 核对文档列明前后行为、源码定位符、验收项、未完成能力和故障反馈模板；后续按问题编号新建修订记录，保留旧快照。
- 本次整理不修改业务代码；源码仍未提交/推送/烧录，完整固件及实板验证状态不变。

## 2026-09-06：功能分层与人工维护入口（当前）

- 新增 `docs/development/README.md`、keyboard/piano/rhythm 功能卡、shared 合同与变更模板，根 AGENTS/AI_DEVELOPMENT/docs 索引已接入。文档 ID 稳定，槽位仅为显示顺序；当前代码仍共享 app_main 和音频 worker。
- 新增可编辑 SVG/PNG 功能地图，以及按文件使用的 `.clang-format`；没有迁移或格式化业务源码，没有引入新功能框架。
- 验证：本地文档链接、SVG XML/箭头/几何/构图及 PNG 目视检查通过；clang-format 19.1.2 配置解析通过；完整宿主 CTest 66/66。
- 下一步：新增第四功能前按模板确定输入/资源/进入退出合同，再分步设计单一活动功能状态和处理器拆分。十几个功能的选择交互、资源容量和自动化隔离仍未实现。

## 2026-09-06：三功能审核与兼容优化（当前）

- 复核原版键盘/PTT/传输、钢琴/曲目、节拍器/鼓机及共享配置、NVS、音频仲裁、睡眠和构建入口。报告：`docs/review/input-music-audit-2026-09-06.md`。
- 新修复：音乐槽调速失败后泄漏原版旋钮动作、未知 MUS1 调式被接受、音乐保存 ACK 混用播放结果；初始化扬声器失败时也返回真实保存状态。
- 优化：七类音乐命令共用私有入队门闸，保留 FIFO/容量/松键补偿；包络步长移至配置更新时缓存，8 组共 768000 样本 PCM 指纹与优化前一致。
- 验证：宿主构建与完整 CTest 66/66；生产函数单线程替身的队列/槽位失败路径通过；ESP32-S3 目标编译器对五个相关源文件语法检查通过，非完整镜像构建。
- 未完成：T3 复现、实际电流/长时堆栈与并发测试；存储曲目读取未形成恢复播放能力，96 事件内置曲目不能写入外部 27 事件 SEQ1 格式。暂不扩展存储合同或修改 GPIO8 生命周期。
- 前轮修改继续保留。尚未提交、推送、运行 CI 或烧录；发布前仍需完整固件构建和 companion/实板回归。

## 2026-09-06：输入与音乐专项审计

- 修复鼓机跨帧累计偏拍、鼓声与钢琴键号混淆、内部曲目越界校验、暂停持续音和 click 节拍器 6/8 间隔；五类问题均有修复前失败的宿主测试，最终完整 CTest 66/66 通过。
- K8 临时发声开关改读 worker 已加载曲目状态，避免待处理请求已被取走后判断失效；第二轮已通过目标语法检查，完整构建/实板仍待验证。
- 删除未读取的音乐命令 drum 标志和混音冗余计数；未改 GPIO、供电或外部协议。CodeGraph 已同步，差异检查通过。
- T3 默认是配置驱动的语音编辑快捷键，误输入尚未复现；待确认实际映射、输出类型和触发时点。暂停仍保留 I2S 会话，实际功耗与长时堆/栈趋势未测量。
- 审计与后续策略：`docs/review/input-music-audit-2026-09-06.md`。全量固件构建交 GitHub Actions，本轮未提交、推送或烧录。

## 2026-09-05：音乐音量旋钮边界

- 槽位 2（钢琴）旋转编码器继续调节音乐音量，范围保持 5%—100%，每格 5%。
- 槽位 3（鼓机）仅在《欢乐颂》序列实际播放时改为调节音乐音量；未播放曲目时仍调节鼓机速度，其他功能不受影响。
- 验证：宿主构建成功，完整 CTest 66/66 通过，`git diff --check` 通过；实板音量和听感仍待验证。

## 2026-09-05：修复钢琴重叠按键断音

- 根因：`MusicSynth` 原先仅有 3 个 voice；按键释放采用衰减尾音，前一个键未完全松开时会继续占用 voice，导致后续重叠按键被覆盖。
- 修改：`components/keyboard/include/keyboard/music_synth.h` 将最大 voice 数扩展为 8，覆盖全部实体键；混音仍按固定最大 voice 数保留削波余量。
- 测试：新增 `music_synth_tests` 的 8 键重叠发声覆盖；`cmake --build build-host --parallel` 成功，`ctest --test-dir build-host --output-on-failure` 通过 66/66，`git diff --check` 通过。
- 未验证：ESP-IDF 构建、烧录和实板听感仍待后续独立验证。

## 2026-09-05：固件 Release 增加离线 ZIP

- `.github/workflows/firmware-release.yml` 在 ESP-IDF 构建和清单生成后新增固定四文件 ZIP：`bootloader.bin`、`partition-table.bin`、`easy_input_keyboard.bin` 与 `firmware-manifest.json`。
- ZIP 同时加入构建产物、构建证明和 GitHub Release 资产；原有单文件资产与 `SHA256SUMS.txt` 保留不变，兼容现有下载和校验流程。
- 验证：工作流 YAML 由 Python `yaml.safe_load` 解析通过，`git diff --check` 通过；尚未触发 GitHub Actions。

## 2026-09-03：修正《欢乐颂》展开段八分音符时值

- 按用户复核的简谱，展开段第 2、3 小节中经过的 `3 4` 由四分音符（十六分拍 4）改回八分音符（十六分拍 2）；A/B/D 句的附点 `6`、八分 `2`、二分 `8` 与 C 句低音 5 均核对无误，事件总数维持 96。
- `music_sequence_tests` 新增展开段八分音符对断言，完整宿主 CTest 66/66、`git diff --check` 通过；随 `firmware-v0.2.18` 交 GitHub Actions 构建，未烧录。

## 2026-09-03：K6 停止 K8 曲目与播放指示灯复位

- K6 在 K8《欢乐颂》播放中由暂停/继续改为停止：立即结束曲目并释放当前音符，K8 退回播放前状态（鼓机键可用）；没有曲目时保留鼓机暂停/继续语义。播放中第三颗灯维持低亮度慢闪（用户确认保留）。
- 切换槽位离开鼓机（鼓机→钢琴）时结束仍在播放/暂停的曲目；切到键盘槽的原有整段停止路径不变。
- 音频 worker 的音乐请求收尾统一清 `music_sequence_active/paused` 并把序列播放器退回停止态：取消路径（如麦克风抢占、写失败）不再让第三颗灯残留亮态，下一个音乐会话也不会从中途续播旧曲目。
- 宿主 CTest 66/66、`git diff --check` 通过；本地 ESP-IDF 构建本轮被用户拒绝执行，交由 GitHub Actions 验证。未烧录，K6 停止听感与灯光熄灭仍待实板确认。

## 2026-09-03：修正 K8 低音与播放控制

- 内置《欢乐颂》事件增加仅内部使用的 `octave_offset`；展开段中的低音 `5` 以 `degree=4, octave_offset=-1` 计算为 G3，不再错误播放为当前八度 G4。外部 `SEQ1` 继续拒绝非零八度偏移，旧 wire 不变。
- K5/K7 在 K8 曲目播放时先停止曲目，再切换鼓机拍号/顺序循环；K6 保持曲目优先的暂停/继续。切到功能一时清空待曲目和控制命令，避免切回功能二续播旧曲目；停止路径按八度偏移释放当前音符。
- 静态差异检查通过；按用户要求不再执行本地构建，发布 tag 后由 GitHub Actions 验证。未烧录。

## 2026-09-03：完善《欢乐颂》展开段与反复

- K8 内置曲目按用户简谱和联网核对结果改为 `前两句 + 展开段/收束段 + 展开段/收束段反复一次`，共 98 个主旋律事件；图片伴奏不加入。
- 4/4 时值继续使用十六分拍单位：普通四分音符 4、八分音符 2、二分音符 8，附点收束音按 6/2/8；内部事件存储扩展到 128，外部 `SEQ1` 63 字节协议仍保持 27 事件上限。
- 力度按实际小节拍位自动生成，不保存逐音符力度表；频率继续复用功能 2 的根音/调式/移调公式。
- 完整宿主 CTest 66/66、`git diff --check` 通过；本轮未重新执行 ESP-IDF 构建或烧录，发布前交由 GitHub Actions 验证。

## 2026-09-02：完善《欢乐颂》四句主旋律

- 内置 K8 曲目改为图片对应的四句 4/4 主旋律，共 60 个音符；只保留主旋律，不加入图片下方伴奏。`3. 2_ 2 -` / `2. 1_ 1 -` 按 `6/2/8` 十六分拍时值表达附点四分音符、八分音符和二分音符。
- 外部 `SEQ1` 63 字节协议仍限制 27 事件；仅扩大固件内部曲目存储到 64 事件，K8 直接走内部播放器，不增加 PCM 或频率缓存。
- `music_sequence_tests` 定向通过，`git diff --check` 通过；本轮完整宿主回归待执行，未烧录。

## 2026-09-02：内置曲目力度按节拍生成

- 移除《欢乐颂》逐音符 `kVelocity[]` 数组，改为按每小节 8 个八分音符自动生成力度：第一拍 100、其它正拍 85、弱拍 70；音符仍复用功能 2 的频率公式。
- `music_sequence_tests` 定向测试通过，`git diff --check` 通过；本轮未重新执行 ESP-IDF 构建或烧录，后续发布前由 GitHub Actions 验证。

## 2026-09-02：K8 曲目暂停与低亮度播放提示

- K6 现在按当前内容暂停/恢复：鼓机循环时暂停鼓机，K8《欢乐颂》播放时暂停/继续序列播放器。
- K8 播放期间仅让第三颗 WS2812 以低亮度慢闪提示，暂停时保持更暗静态灯；不使用全灯高亮，避免刺眼。播放状态由音频 worker 原子发布给 LED owner。
- 宿主 CTest 66/66、`git diff --check`、ESP-IDF 5.5.5 / ESP32-S3 全量构建通过；应用镜像 `0x194d90`，最小 App 分区余量 47%。未烧录，实板暂停恢复和灯光舒适度仍待验证。

## 2026-09-02：鼓机槽 K8 播放《欢乐颂》

- 鼓机槽 K8 新增内置《欢乐颂》播放入口，复用现有 `builtin_music_sequence`、`MusicSequencePlayer` 和 `MusicSynth` 的频率/时值计算，不增加 PCM 资源；播放前由音频 worker 清理鼓机声部，避免旋律与鼓声硬叠。
- 音乐配置关闭时，K8 仅临时允许本次曲目音符发声，不改写已保存的 `music_v1` 配置；K1—K7 鼓机控制保持不变。
- 宿主 CTest 66/66、`git diff --check`、ESP-IDF 5.5.5 / ESP32-S3 全量构建通过；应用镜像 `0x194bb0`，最小 App 分区余量 47%。未烧录，K8 实板旋律、音准、时值和切换听感仍待验证。

## 2026-09-02：鼓机拍号与循环切换修正

- K5 改为固定循环 `4/4 → 3/4 → 6/8 → 2/4`，首次按下即启动 `4/4`；6/8 按八分音符推进，其余拍号按四分音符推进。
- K6 继续暂停/恢复，K7 控制四种鼓声顺序循环（Kick→Snare→Hi-hat→Click）；切换拍号或循环时保留当前音频倒计时，不额外强制触发一声，降低播放中切换的突发杂音风险。
- 宿主 CTest 66/66、`git diff --check` 和 ESP-IDF 5.5.5 / ESP32-S3 全量构建通过；应用镜像 `0x194b50`，最小 App 分区余量 47%。未烧录，实板节拍、旋钮方向、暂停和切换听感仍待验证。

## 2026-09-02：功能槽位输入隔离修复

- 根因：钢琴/鼓机槽的音频处理失败时返回 `false`，主流程随后回退到默认 Keymap，触发第一槽 Host Action。
- 修复：只要处于音乐或鼓机槽，KEY1—KEY8 无条件由当前槽消费；切槽前清理已持有的键盘/HID 状态，避免旧 Host Action 残留。
- 验证：宿主 CTest 66/66、差异检查通过；未烧录、未推送。已有进入 USB/BLE FIFO 的旧 Host Action 仍需实板确认是否存在迟到触发。

## 2026-09-02：功能槽位常亮降亮度

- 槽位常亮背景色从 `{0,18,24}` 降为 `{0,5,7}`；短时输入、连接和配置提示不受影响。
- 仅修改 LED 背景渲染，尚未烧录或实板观察。

## 2026-09-02：鼓机节拍、循环与速度控制

- 鼓机槽位内新增独立 `DrumSequencer`：Key5 依次切换 4/4 节拍、3/4 节拍、关闭；Key7 开/关 Kick→Snare→Hi-hat→Click 顺序循环；Key6 暂停/继续当前循环；旋钮每格以 5 BPM 调速，范围限制为 20—300 BPM。
- 循环时钟由唯一 PCM/I2S worker 按渲染帧推进，避免主任务计时抖动及与 GPIO8 共享电源、音频仲裁并发；离开鼓机或创建新音乐会话会清除循环，钢琴与键盘模式不消费这些控制。
- 新增 `drum_sequencer_tests`；重新配置后宿主 CTest 66/66 通过，`git diff --check` 通过。ESP-IDF 5.5.5 / ESP32-S3 全量构建通过，应用镜像 `0x194a60`，最小 App 分区余量 47%。未烧录，实板按键、旋钮方向、暂停与听感仍待验证。

## 2026-09-02：USB 描述符与固定资源审计

- 修复 Windows Code 10 的直接原因：键盘 LED Output 恢复 `5+3=8 bit` 对齐；将 `SEQ1/0x17` 的 63 字节 Feature/Output 移入 Vendor Collection，并补齐 `0x13` 的 16 字节 Output 声明，保持与回调合同一致。
- BLE 配置 GATT 写回调改用 63 字节定长数组，拒绝超长帧，消除远端长度触发的堆分配与碎片化路径；删除一条 `uint8_t > 255` 永远为假的无效校验。
- 宿主 CTest 65/65、`git diff --check`、ESP-IDF 5.5.5 / ESP32-S3 全量构建均通过；应用镜像 `0x194190`，最小 App 分区余量 47%。未烧录、未验证 Windows 重新枚举或 Studio 写入。
- 强度复核：P1 为 0；待验证 P2 为 TinyUSB 鼠标宏未展开进位账本，以及音频采集 4 KiB 任务栈需要实板长时 high-water 证据。

## 2026-09-02：修复外部诊断状态请求报告类型

- 根因：状态请求 `0x13` 在 USB 描述符中声明为 Feature Report，但现有 Flasher Go HID 库只能发送 Output Report，导致请求无法进入 Maker 回调。
- 修复：将 Vendor 状态请求 `0x13` 声明为 16 字节 Output Report；回调同时兼容 Feature/Output，保留原请求格式、状态响应和 512 字节预算。
- 验证：`config_status_tests`、`status_hid_protocol_tests` 通过；需用新固件重新构建/烧录后才能验证 Flasher 实机回报。

## 2026-09-01：外部烧录器复用板级状态遥测

- 保持现有 Vendor HID 状态报告和 512 字节预算不变；状态 JSON 已提供 `diag.last`、`diag.in_evt`、`diag.enc_step`、电池/供电字段，供外部工具做初步排查。
- 未新增常驻诊断服务或大型资源；具体按键业务动作仍由固件配置决定。
- Flasher 端读取与动态轮询已完成静态联调，真实 Windows HID 回报和实板功能仍待验证。
- Vendor HID 状态请求增加 1 bit `diagnostics` 标志；请求带该标志时沿用 `diag` 阶段输出详细输入/旋钮字段，不改变报告大小和旧请求兼容。

## 当前目标

- 维护 `my-host-action-practice` 的可复现宿主测试与 ESP-IDF 5.5.5 构建门禁。
- 将八个实体键作为离线钢琴接入既有 `SpeakerOutput`，旋钮调节音量，不新建并行 I2S 所有者。
- 实现旋钮短按的环形功能切换，替代高频桌面端切换；当前槽位为键盘/音乐。
- 修复已发布版本中旧 NVS 配置导致旋钮短按不进入环形切换的问题。
- 调整交互：短按恢复原有旋钮动作；3 s 时灯光预览下一个槽位，3—6 s 松开切换；6 s 长按进入配置模式。
- 功能切换反馈使用 5 颗 WS2812 的低位位图：槽位编号从 1 开始，bit0 对应第一颗灯；例如 1=00001、2=00010、3=00011，环回仍由 `EncoderFunctionRing` 保证。
- 本轮将环形功能扩展为键盘/音乐/鼓机三槽；鼓机 KEY1—KEY4 分别合成 Kick、Snare、Hi-hat、Click，继续复用 GPIO8 共享电源和唯一 I2S owner。
- `SEQ1` 增加 Pause/Resume 控制，播放或内置曲目接收时保存 `song_v1`；启动仅校验并恢复曲目到 RAM，不自动发声。
- 默认功能语义：上电固定为第一个键盘功能；音乐配置只恢复参数，不自动把设备切到第二槽。
- TODO：定位既有 `audio_host` 电脑端语音 companion 后，评估可配置语音规则“选中当前输入框”与“将当前选中文本优化为邮件”；该能力属于电脑端剪贴板/前台窗口自动化，不在 Maker 固件内重复实现。

## 已完成

- 修复旋钮切换状态错位：启动默认从键盘槽开始，旧配置中的 `settings`/`scroll_axis_toggle` 按压动作也迁移到 `function_cycle`；鼓槽位的非鼓键释放不再回落到普通键盘路径。

- 多键发声首轮修复：音乐会话启动时清空上一会话的按键位图与边沿队列；同一按键的重复同状态边沿不再重复触发包络；三声部混音按最大复音数预留动态余量，降低和弦削波失真。
- `SEQ1/0x17` 时序测试已按协议实际语义修正：时值字段以十六分拍数编码，`4` 代表一拍；两个 120 BPM 事件共需 48000 帧后结束。
- `SEQ1` 增加向后兼容的力度字段：音符字节高 4 位为 16 档力度、低 3 位仍为音阶编号；旧版 0—7 音符编码继续按满音量播放。Studio 支持 `音符:时值@力度`，例如 `5:8@60`。
- 新增公版内置《欢乐颂》测试曲目：固件保存音符/时值/力度数组，Studio 通过 `SEQ1` Builtin 命令一键请求播放；自定义简谱编辑入口继续保留。
- 新增 `EncoderFunctionRing` 纯逻辑环形索引与 `function_cycle` 动作；旋钮短按默认循环功能，扬声器构建在键盘/音乐两槽间切换，无扬声器构建保持单槽兼容；宿主 CTest 65/65 通过。
- 新增设计文档 `docs/design/encoder-profile-switching.md`：建议以 4 个本地 profile 槽位承载日常、写作、音乐和备用配置，旋钮短按循环切换，3 秒长按保留现有配置入口；定义 `profiles_v1` NVS 原子提交、失败回滚、LED/状态反馈及旧 `S3C` 兼容边界。当前仅完成方案设计，尚未修改运行时代码。

- Music Config v1 已接入真实 USB HID：`0x16/MUS1` 同时声明 Feature/Output 63 字节报告，严格解码后由主任务独立保存 NVS `music_v1`，并传给 `SpeakerOutput` 的唯一 I2S 所有权模型。正常启动会恢复已保存的音乐配置。
- 钢琴模式已接入 KEY1—KEY8 音符边沿和旋钮音量（每格 5%，范围 5%—100%）；开启音乐模式时按键不会再同时发送旧键盘动作。配置应用由 I2S worker 的下一帧完成，避免主任务并发修改合成器。
- 钢琴输入新增“边沿队列 + 最终按键状态”双保险：队列满时仍发布 8 键最终状态，I2S worker 下一帧逐键校正遗漏的 release，不使用超时截断，因此保留长按、和弦与连贯弹奏。
- Music Config v1 的纯逻辑基础已加入：`MusicSynth` 覆盖音高、音色、ADSR、三声部、节拍和离线调音估计，`music_config_protocol_tests` 固定 `MUS1` 63 字节 wire 的 round-trip 与 fail-closed 拒绝；USB/NVS 工作区接线与 I2S 接入的验收状态见本文件后续条目。
- 钢琴已接入现有扬声器工作任务：KEY1—KEY8 默认 C4—C5，按下/松开经短命令队列交给唯一 PCM 渲染者；旋钮每格调节 5% 音量并限制在 5%—100%。音乐播放继续走既有 I2S1、GPIO8 电源握手和 `AudioIoArbiter`，麦克风请求会取消音乐。
- 新增 `scripts/build_firmware_manifest.py` 与 `.github/workflows/firmware-release.yml`：仅在未来 `firmware-v*` 标签上完成既有宿主测试和 ESP-IDF 5.5.5 / ESP32-S3 构建后，生成三段镜像清单、SHA-256、构建溯源和 GitHub Release 资产，供独立桌面烧录器校验；本轮只完成 Python 编译、真实本地构建产物清单生成和 YAML 解析，未推送、未触发 Actions、未发布或烧录。
- `read_source()` 使用二进制模式读取资源，并对文本源码统一 CRLF/LF，消除 Windows 平台差异。
- 新增 `.github/workflows/firmware-validation.yml`，执行宿主 CMake 编译、CTest 全量测试和 ESP32-S3 固件构建。
- 已创建个人 Fork：`FreeCodeCampXYG/easy-input-maker`；官方仓库保留为 `upstream`。
- 已创建并推送注释标签 `my-host-action-practice-v1`，指向提交 `9daf462`。
- 工作流补充上传同一提交对应的 ESP32-S3 固件 bundle，供统一硬件直接下载烧录。
- GitHub Release `my-host-action-practice-v1.1` 已发布，包含 5 个固件/刷写附件及 Changed、Flashing 说明。
- T03 纯逻辑 Host Action 已完成：UUID fail-closed 校验、Keymap/事件与共享 payload 编码已加入；宿主测试 58/58 通过，USB/BLE 适配未改。
- T06 Vendor HID 接入已完成：USB/BLE 复用共享 Host Action payload 并沿用 USB-first 单通道；宿主测试 58/58 通过，ESP-IDF 与真机发送仍待验证。
- T08 能力声明已完成：状态 JSON 各 capabilities 路径均有唯一布尔 `host_action_v1:true`；最终 BLE wire 极值最大 502/512 字节，宿主测试 58/58 通过。
- T09 软件侧总验收已完成：重新配置后 CTest 发现/执行/通过均为 60，失败 0；ESP-IDF v5.5.5 / esp32s3 默认构建成功，应用镜像 1,640,384 bytes，App 分区余量 48%。
- 回归复核修复：将 Host Action 与旧 `FirmwareEventKind::AppCommand` 分离，避免 history/settings/profile 等旧 AppCommand 被误编码为 Host Action；定向 4 项与完整 60 项宿主测试均通过。
- 回归修复后的 ESP-IDF v5.5.5 / esp32s3 增量构建成功，应用镜像 `build/easy_input_keyboard.bin` 为 1,640,464 bytes，App 分区余量 48%。
- CodeGraph daemon 已确认启用文件 watcher 自动同步；本次 `codegraph status` 报告索引最新（4,567 nodes / 17,586 edges）。
- T10 烧录前审计已收口：`host_test/CMakeLists.txt` 中重复的 `host_action_protocol.cpp` source 项已最小去重；重新配置后 CTest 60/60 通过，ESP-IDF v5.5.5 / esp32s3 构建成功，`SAFE_TO_REQUEST_FLASH = YES`。
- 已确认开发板正常模式 USB/HID 枚举为预期 VID/PID `303A:1006`；当前项目只提供 HID，正常模式没有 CDC 串口是预期行为。
- 已完成一次经人工确认的完整工程烧录：下载模式唯一端口与 ESP32-S3 身份在写入前复核，bootloader、分区表和应用镜像写入后均通过 hash 校验；未执行整片擦除。使用者已通过板上电源开关完成完全关机再正常开机，Windows 枚举到预期 EasyInput AI USB/HID 正常模式。
- 本轮重新构建并烧录当前工作区：完整 CTest 61/61 通过，ESP-IDF v5.5.5 / esp32s3 构建成功，应用镜像 1,640,464 bytes、App 分区余量 48%；使用者确认完全关机再正常开机后，预期 USB/HID 集合再次枚举正常。

## 验证

- 本轮 `cmake --build build-host --parallel` 与 `ctest --test-dir build-host --output-on-failure`：65/65 通过；`git diff --check` 通过。ESP-IDF/实板验证尚未执行。

- 本轮定向 `music_synth_tests` 与 `music_live_control_tests`：2/2 通过；宿主增量构建成功，`git diff --check` 通过。
- 本轮完整 CTest：64/64 通过；覆盖 `MUS1`、`SEQ1`、音乐合成、实时按键状态和既有固件回归。
- 本轮完整 CTest：65/65 通过；Studio `go test ./...` 与前端 `npm run build` 通过。ESP-IDF 命令已进入受管环境，但项目 build 使用了旧 Python 环境，未取得本轮源码重编译证据。
- Studio `go test ./...` 与前端 `npm run build` 均通过。
- 内置曲目覆盖测试通过：曲目编号校验、事件容量限制和首音符内容均已加入宿主测试。
- GitHub Actions run `33360808188` 的固件验证通过；标签 `firmware-v0.2.3` 已创建并发布，Release 已补齐 5 个固件/清单资产。标签工作流最后一步因同名 Release 已预先创建而失败，不影响前置构建、证明和资产上传。

- 本轮 `cmake --build build-host` + `ctest --test-dir build-host --output-on-failure`：63/63 通过；新增 music live control 覆盖八键映射与音量边界。
- 本轮串音修复：定向 `music_live_control_tests` 通过，覆盖最终按键状态和遗漏 release 的差异检测；ESP-IDF 全量构建按用户偏好留待 GitHub Actions 验证。
- 独立 `build-music/`：ESP-IDF v5.5.5 / esp32s3 构建成功，应用镜像 `0x192ca0`，App 分区余量 48%；后续确认烧录的三段镜像均通过设备端 hash 校验，自动复位后重新检测到 EasyInput AI BLE 正常模式；音频和 HID 收发功能仍待实板验证。
- 2026-08-31：宿主 CTest 63/63 通过（新增 `music_live_control_tests`）；ESP-IDF v5.5.5 / ESP32-S3 全量构建成功，镜像 `0x192ca0`，App 分区余量 48%。
- 本轮重新配置后的 CTest 实际发现/执行/通过：62/62，失败 0；Host Action 三个专项注册名均存在。课程起点基线为 57，Host Action 三项使历史总数到 60，原有 Music Synth 测试为第 61 项，本轮新增 Music Config wire 测试为第 62 项。
- 本轮 ESP-IDF v5.5.5 / esp32s3 构建成功；应用镜像 `0x190810`，App 分区余量 48%。
- 本地 `ctest --test-dir build-host --output-on-failure`：60/60 通过。
- 本轮 ESP-IDF v5.5.5 / esp32s3 `idf.py build`：成功；应用镜像 `0x190810`，App 分区余量 48%。
- GitHub Actions run `33275780034`：宿主编译、57 项测试和 ESP-IDF 固件构建均通过。

## 已知边界

- 多键修复尚未在 ESP-IDF 构建、烧录或实板听感上验证；是否仍有功放供电、I2S DMA 或扬声器本体噪声，需要实板证据区分。
- `SEQ1` 力度扩展尚未在 ESP-IDF 构建、烧录或实板动态听感上验证；报告长度和旧编码兼容性已有宿主/Studio 测试覆盖。

- 离线钢琴已进入 `SpeakerOutput` 的唯一 I2S1 所有权模型；USB `0x16` 与独立 NVS 的工作区改动尚未做独立端到端验收。本版本已完成普通烧录并检测到正常模式，音量方向、音准、手感、节拍器和麦克风抢占均仍待实板验证。
- 鼓机合成、SEQ1 暂停/继续、`song_v1` NVS 和三槽切换尚未在 ESP-IDF 重编译、烧录或实板听感上验证；Kick/Snare/Hi-hat/Click 的音色仍属于固件合成初版。
- 本轮串音修复尚未经 ESP-IDF 全量编译、烧录或实板验证；在 GitHub Actions 产物验证后，仍须重新验明设备并取得确认才可写入。
- 新的固件 Release 流程尚未在 GitHub Actions 或真实设备验证；现有 `my-host-action-practice-v1.1` 缺少新 manifest，桌面端会安全地忽略它。
- Actions 日志有 Node.js 20 弃用提示；当前不阻断工作流。
- 尚未执行烧录或实板验证；构建与实板观察仍是不同证据。烧录前仍需进入下载模式、读取芯片/MAC 并获得本轮精确确认。
- Host Action 的真实按键、USB/BLE 实际发送和 App 映射仍未验证；本次正常模式 HID 枚举只能证明设备恢复运行。

## 下一步

- 先建立 EasyInput App、USB HID 或 BLE 的正常模式连接证据，再执行 Host Action 的真实功能验证；如需排查烧录后检查脚本，修复本机 Git Bash 对受管 Python 的路径发现，不得为此重新烧录。
- 经使用者确认目标设备后，再烧录并验证八个音符、三键和弦、音量边界、麦克风抢占与原有扬声器资产播放恢复；不要将构建成功写成实板发声通过。
- 可视化烧录工具排查以 `docs/getting-started/visual-flasher-diagnostics.md` 的状态机为准；记录身份、镜像、写入、恢复和业务观察各自的证据，且不在公开记录中保留完整 MAC、端口或原始串口日志。
