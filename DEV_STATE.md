# EasyInput Maker 开发状态

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
