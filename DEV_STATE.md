# EasyInput Maker 开发状态

## 当前目标

- 维护 `my-host-action-practice` 的可复现宿主测试与 ESP-IDF 5.5.5 构建门禁。
- 将八个实体键作为离线钢琴接入既有 `SpeakerOutput`，旋钮调节音量，不新建并行 I2S 所有者。

## 已完成

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

- 离线钢琴已进入 `SpeakerOutput` 的唯一 I2S1 所有权模型；USB `0x16` 与独立 NVS 的工作区改动尚未做独立端到端验收。本版本已完成普通烧录并检测到正常模式，音量方向、音准、手感、节拍器和麦克风抢占均仍待实板验证。
- 本轮串音修复尚未经 ESP-IDF 全量编译、烧录或实板验证；在 GitHub Actions 产物验证后，仍须重新验明设备并取得确认才可写入。
- 新的固件 Release 流程尚未在 GitHub Actions 或真实设备验证；现有 `my-host-action-practice-v1.1` 缺少新 manifest，桌面端会安全地忽略它。
- Actions 日志有 Node.js 20 弃用提示；当前不阻断工作流。
- 尚未执行烧录或实板验证；构建与实板观察仍是不同证据。烧录前仍需进入下载模式、读取芯片/MAC 并获得本轮精确确认。
- Host Action 的真实按键、USB/BLE 实际发送和 App 映射仍未验证；本次正常模式 HID 枚举只能证明设备恢复运行。

## 下一步

- 先建立 EasyInput App、USB HID 或 BLE 的正常模式连接证据，再执行 Host Action 的真实功能验证；如需排查烧录后检查脚本，修复本机 Git Bash 对受管 Python 的路径发现，不得为此重新烧录。
- 经使用者确认目标设备后，再烧录并验证八个音符、三键和弦、音量边界、麦克风抢占与原有扬声器资产播放恢复；不要将构建成功写成实板发声通过。
- 可视化烧录工具排查以 `docs/getting-started/visual-flasher-diagnostics.md` 的状态机为准；记录身份、镜像、写入、恢复和业务观察各自的证据，且不在公开记录中保留完整 MAC、端口或原始串口日志。
