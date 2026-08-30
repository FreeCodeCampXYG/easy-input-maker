# EasyInput Maker 开发状态

## 当前目标

- 维护 `my-host-action-practice` 的可复现宿主测试与 ESP-IDF 5.5.5 构建门禁。

## 已完成

- Music Config v1 的纯逻辑基础已加入：`MusicSynth` 覆盖音高、音色、ADSR、三声部、节拍和离线调音估计，`music_config_protocol_tests` 固定 `MUS1` 63 字节 wire 的 round-trip 与 fail-closed 拒绝；尚未接入 USB/NVS/I2S。
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

- 本轮重新配置后的 CTest 实际发现/执行/通过：62/62，失败 0；Host Action 三个专项注册名均存在。课程起点基线为 57，Host Action 三项使历史总数到 60，原有 Music Synth 测试为第 61 项，本轮新增 Music Config wire 测试为第 62 项。
- 本轮 ESP-IDF v5.5.5 / esp32s3 构建成功；应用镜像 `0x190810`，App 分区余量 48%。
- 本地 `ctest --test-dir build-host --output-on-failure`：60/60 通过。
- 本轮 ESP-IDF v5.5.5 / esp32s3 `idf.py build`：成功；应用镜像 `0x190810`，App 分区余量 48%。
- GitHub Actions run `33275780034`：宿主编译、57 项测试和 ESP-IDF 固件构建均通过。

## 已知边界

- Music 的真实 I2S 不能以并行新驱动接入：默认 Speaker Assets 产品链已独占 I2S1。必须先合并到 `SpeakerOutput` 的唯一音频所有权模型并定义麦克风抢占，才可接入 USB `0x16`、独立 NVS 和真实扬声器/麦克风；当前不能宣称离线钢琴或调音器可在设备运行。
- 新的固件 Release 流程尚未在 GitHub Actions 或真实设备验证；现有 `my-host-action-practice-v1.1` 缺少新 manifest，桌面端会安全地忽略它。
- Actions 日志有 Node.js 20 弃用提示；当前不阻断工作流。
- 尚未执行烧录或实板验证；构建与实板观察仍是不同证据。烧录前仍需进入下载模式、读取芯片/MAC 并获得本轮精确确认。
- Host Action 的真实按键、USB/BLE 实际发送和 App 映射仍未验证；本次正常模式 HID 枚举只能证明设备恢复运行。
- Music Synth 当前仅完成音高/调式/相位增量纯逻辑与宿主测试，尚未接入持续扬声器、节拍器、调音器或 Music Config v1；不能将本轮烧录视为音乐功能完成。

## 下一步

- 先建立 EasyInput App、USB HID 或 BLE 的正常模式连接证据，再执行 Host Action 的真实功能验证；如需排查烧录后检查脚本，修复本机 Git Bash 对受管 Python 的路径发现，不得为此重新烧录。
