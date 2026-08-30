# T10 · Host Action v1 烧录前只读修改范围审计

> 本节点只读审计，不修改生产代码、配置、测试或设备。审计基线为课程起点提交 `743368a` 到当前 `HEAD` 及全部 staged、unstaged、untracked 内容。

## 审计命令与基线

- `git merge-base --is-ancestor 743368a HEAD`：退出 0，课程起点是当前分支历史祖先。
- `git status --porcelain=v1`、`git diff --name-status HEAD`、`git diff HEAD --unified=0`：已覆盖 staged（无）、unstaged 与 untracked 内容。
- `git check-ignore -v build build-host ...`：`build/`、`build-host/` 被根 `.gitignore` 忽略；`.codegraph` 数据库由其局部 `.gitignore` 忽略。
- `git diff --check`：通过。
- `easyinput-board-cy check_board_baseline.py .`：1 PASS / 2 WARN / 0 FAIL；WARN 为未指定构建分支和条件编译引脚识别限制，未作为硬件已验证结论。

## 全量文件分类

| 分类 | 文件 | 结论 |
|---|---|---|
| Host Action 生产实现 | `components/keyboard/{include/keyboard/host_action_protocol.h,src/host_action_protocol.cpp,src/config_payload.cpp,src/keymap.cpp,src/config_status.cpp,include/keyboard/{keymap.h,config_status.h},CMakeLists.txt}`；`main/app_main.cpp`；`main/platform/{usb_hid.*,ble_hid.*}` | 合同相关；已读 diff |
| 宿主测试 | `host_test/{host_action_tests.cpp,CMakeLists.txt,config_status_tests.cpp,firmware_source_contract_tests.cpp}` | 合同/八键/能力/路由覆盖；重复 source 已最小去重并重新配置验证 |
| project-flow-cy 记录 | `flow/{decisions.md,plan.md,进展.md,tasks/T03*,T04*,T06*,T08*,T09*,T12*,T13*}`、`DEV_STATE.md` | 课程与状态记录 |
| CodeGraph 本机索引 | `.codegraph/.gitignore`（数据文件被局部 ignore） | 本机元数据，不应提交 |
| 被忽略生成物 | `build/`、`build-host/` | CMake/ESP-IDF 二进制、缓存与本机路径；未进入 Git 范围 |
| 课程起点后既有提交 | `.github/workflows/firmware-validation.yml`、先前 `DEV_STATE.md`/测试读取修复 | 位于 `743368a..HEAD`，不是本轮 Host Action 工作区实现 |

## 冻结合同与禁止范围矩阵

| 项目 | 结果 | 证据 |
|---|---|---|
| GPIO、BOOT、GPIO8 共享电源未被 Host Action 改变 | PASS | diff 未涉及 `board_pins.h`、`gpio_keys.*`、`peripheral_power.*`；板级扫描 0 FAIL |
| v2/esp32s3、Flash 分区、容量与 sdkconfig 不变 | PASS | 根 CMake/`partitions.csv`/`sdkconfig.defaults` 无 diff；构建 `CONFIG_IDF_TARGET="esp32s3"`、Flash `16MB` |
| USB/BLE 身份、HID 描述符、GATT 标识与地址策略不变 | PASS | `usb_hid.*`/`ble_hid.*` diff 仅增加 Host Action 队列入口；VID/PID、名称、描述符、GATT 标识无 diff |
| Host Action wire 合同 | PASS | 共享编码固定 `0x11`/`0x05`/`0`/`1`/`36`、UUID `[4..39]`、余量清零；状态 `0x04` 不变；60/60 宿主测试与 v5.5.5 构建通过 |
| 单通道 USB-first、失败不跨通道补发 | PASS | `dispatch_firmware_event()` 既有 USB epoch 分支失败后 return；Host Action 沿用同一路由 |
| 无应用路径/Bundle ID/名称/图标/启动参数映射 | PASS | 全仓示例 UUID 仅在 `host_test/host_action_tests.cpp`；生产源码只保存/编码 UUID 合同 |
| 新日志不泄露用户内容、UUID、应用信息或设备唯一标识 | PASS | 新日志只含固定 `host_action` 文本与固定输入源名；diff 未记录 UUID/应用路径/自定义文本 |
| 无关复制、粘贴、快捷键、固定文字、音频、电源功能 | PASS | 独立 HostAction 事件修复保留旧 AppCommand；60/60 回归及 ESP-IDF 构建通过 |
| build/build-host 与本机路径未进入提交范围 | PASS | 根 `.gitignore` 命中；状态中未出现这些目录内容 |
| 宿主构建源清单无重复 | PASS | `host_action_protocol.cpp` 仅保留一个 `easy_input_keyboard_core` source 项；重新配置、构建和 CTest 60/60 通过 |

## 结论

`SAFE_TO_REQUEST_FLASH = YES`

唯一阻断项已在 `host_test/CMakeLists.txt` 最小去重；重新配置、完整 CTest 60/60、ESP-IDF 5.5.5 / esp32s3 构建、`git diff --check` 和板级扫描均通过。随后已按人工确认完成一次完整工程烧录，正常模式 USB/HID 已枚举到预期 VID/PID；未删除、回退或覆盖其他既有改动。Host Action 的 App/真机功能验证仍不属于本审计结论。
