# T10 · Host Action v1 烧录前只读修改范围审计

## 范围与结论

- 审计对象是课程起点 Git 基线到当前工作区的全部 staged、unstaged、untracked 内容，以及被 Git 忽略的 `build/`、`build-host/`，不是只看上一节点增量。
- 本节点不修改生产代码、配置、测试或硬件身份，不访问设备、串口或 App，不烧录；审计完成后只更新根级 `flow/`。
- 八项禁止边界全部为 `PASS`，32 个写入前变化文件与 2 个生成目录均有可追溯归属，没有无关或无法解释项。
- **SAFE_TO_REQUEST_FLASH = YES**。这只表示可以进入“请求烧录授权”环节，不是烧录授权，也不代表设备身份、烧录或真机功能已经验证。

## 可靠基线

- `git rev-parse HEAD`、`git rev-parse origin/main` 与 `git merge-base HEAD origin/main` 均为 `34087cd40d24d23579da0357973ebc1a37e7ce7c`，分支为 `main`，相对 `origin/main` 为 `+0/-0`。
- T01 把课程起点记录为公开 Maker 的干净 `main@34087cd`；T02 在首次合同写入前进一步记录，当时只有既有 `flow/` 变化，没有 `components/`、`host_test/` 或 `main/` 变化。
- 因此课程起点到当前工作区的基线可由 Git 与 `flow/` 交叉确定，不需要把基线标成 `UNKNOWN`。
- 当前写入前状态：staged 0；unstaged tracked 20；untracked 12；ignored 生成目录 2。
- 上一节点记录的 CTest 结果只作为已存在证据读取：实际发现 59、执行 59、通过 59、失败 0；本审计没有重新运行测试或构建。

## 只读命令

```bash
git status --porcelain=v2 --branch
git rev-parse HEAD
git rev-parse --verify origin/main
git merge-base HEAD origin/main
git diff --cached --name-status
git diff --name-status
git ls-files --others --exclude-standard
git status --short --ignored

git diff -- <每个已跟踪变化文件>
sed -n <相关范围> <每个未跟踪文件>
git diff --numstat HEAD

python3 <easyinput-board-cy>/scripts/check_board_baseline.py --help
python3 <easyinput-board-cy>/scripts/check_board_baseline.py .

git diff --exit-code HEAD -- \
  CMakeLists.txt sdkconfig.defaults partitions.csv \
  components/keyboard/include/keyboard/board_pins.h \
  main/platform/gpio_keys.cpp main/platform/peripheral_power.cpp \
  main/platform/keyboard_audio.cpp main/platform/led_strip_status.cpp \
  main/platform/battery_adc.cpp docs/hardware/easyinput-v2-safety.md

git show HEAD:<身份文件> | sed -n <身份片段> | shasum
sed -n <相同身份片段> <当前身份文件> | shasum

rg <固定示例 UUID／应用映射／隐私日志／本机绝对路径模式> \
  --glob '!build/**' --glob '!build-host/**' --glob '!.git/**'
git check-ignore -v build build-host
git ls-files build build-host
git diff --check
git diff --cached --check
```

上述命令均为只读。板型扫描退出 0，结果为 1 `PASS`、2 `WARN`、0 `FAIL`；两个 `WARN` 是扫描器未选择 `EASY_INPUT_BOARD_V2` 条件分支，分别为 `BUILD_BRANCH_UNSELECTED` 与 `PIN_DECLARATIONS_NOT_FOUND`。本审计没有把工具限制改写成硬件失败，而是继续用可靠 Git 基线、当前 v2 编译定义和源文件逐项比较。

## 全量文件与目录分类（写入审计记录前）

| 状态 | 文件或目录 | 归类 | 实际 diff／内容说明 |
|---|---|---|---|
| unstaged tracked | `components/keyboard/CMakeLists.txt` | Host Action 生产实现 | 只把共享 `host_action_protocol.cpp` 注册进 keyboard 组件 |
| unstaged tracked | `components/keyboard/include/keyboard/ble_status_wire.h` | Host Action 生产实现／512 状态兼容 | 把既有 BLE 详细状态附加逻辑抽成可宿主测试的共享有界编码；不改 GATT 标识 |
| unstaged tracked | `components/keyboard/include/keyboard/config_status.h` | Host Action 生产实现／能力声明 | 共享 fallback 增加唯一布尔 `host_action_v1: true`；512 上限不变 |
| unstaged tracked | `components/keyboard/include/keyboard/keymap.h` | Host Action 生产实现 | 增加 `ActionKind::HostAction`、完整配置值字段和固件事件类型 |
| untracked | `components/keyboard/include/keyboard/host_action_protocol.h` | Host Action 生产实现 | 唯一共享 `0x11/0x05/0/1/36` 与 63 字节容器常量、报告类型和编码接口 |
| unstaged tracked | `components/keyboard/src/config_payload.cpp` | Host Action 生产实现 | 严格解析并保留完整 `host_action:<UUID>`，非法格式返回 `UnknownAction` |
| unstaged tracked | `components/keyboard/src/config_status.cpp` | Host Action 生产实现／512 状态兼容 | 全部能力路径加入布尔声明；按批准规则收紧 speaker probe 专用 firmware 预算并增加保留 current power 的 cycle 回退 |
| untracked | `components/keyboard/src/host_action_protocol.cpp` | Host Action 生产实现 | 规范小写 UUID fail-closed；线上仅复制无前缀 36 字节 UUID，余量由零初始化保留 |
| unstaged tracked | `components/keyboard/src/keymap.cpp` | Host Action 生产实现 | 按下产生一次 Host Action，松开或无效配置返回 `None`；默认 Keymap 未增加 Host Action |
| unstaged tracked | `main/app_main.cpp` | Host Action 生产实现 | 事件类型进入既有 USB 优先路由；新增日志只输出固定 kind 名，不输出 UUID |
| unstaged tracked | `main/platform/usb_hid.cpp` | Host Action 生产实现 | 调用共享编码并进入既有 App Command 队列；描述符和设备身份片段未变 |
| unstaged tracked | `main/platform/ble_hid.cpp` | Host Action 生产实现／512 状态兼容 | 调用同一共享编码；共享 fallback／最终状态附加；Report Map、GATT、名称、身份地址策略未变 |
| unstaged tracked | `host_test/CMakeLists.txt` | 宿主测试 | 注册共享实现和 3 个 Host Action 测试目标 |
| unstaged tracked | `host_test/config_payload_tests.cpp` | 宿主测试 | 合法完整值与大写／长度／连字符／非法字符拒绝覆盖 |
| unstaged tracked | `host_test/config_state_tests.cpp` | 宿主测试 | 无效更新不覆盖上一份有效 Host Action 配置 |
| unstaged tracked | `host_test/config_status_tests.cpp` | 宿主测试 | 能力布尔、speaker probe 16 字节预算、current power／cycle 回退覆盖 |
| unstaged tracked | `host_test/firmware_source_contract_tests.cpp` | 宿主测试 | 共享编码、单通道路由、平台状态共享边界与日志合同检查 |
| unstaged tracked | `host_test/keymap_tests.cpp` | 宿主测试 | 按下一次、松开 `None` |
| unstaged tracked | `host_test/transport_routing_tests.cpp` | 宿主测试 | Host Action 继续使用 `UsbFirst` |
| untracked | `host_test/host_action_capability_status_tests.cpp` | 宿主测试 | 所有状态变体能力唯一布尔值与最终 512 字节边界 |
| untracked | `host_test/host_action_key_bindings_tests.cpp` | 宿主测试 | KEY1—KEY8 逐键解析、完整前缀、一次 press／无 release 事件 |
| untracked | `host_test/host_action_protocol_tests.cpp` | 宿主测试 | 规范 UUID、精确 payload、0x04／0x05 分工和余量归零 |
| unstaged tracked | `flow/decisions.md` | project-flow-cy 记录 | 冻结 v1 合同及能力只属于状态 JSON 的公开决策 |
| unstaged tracked | `flow/plan.md` | project-flow-cy 记录 | T01—T09 里程碑与任务入口 |
| unstaged tracked | `flow/进展.md` | project-flow-cy 记录 | 合同、纯逻辑、八键、发送、能力和软件总验收交接 |
| untracked | `flow/tasks/T01-工程结构与板型只读核对.md` | project-flow-cy 记录 | 课程起点与硬件边界 |
| untracked | `flow/tasks/T02-Host-Action-v1-固定兼容协议.md` | project-flow-cy 记录 | 冻结合同 |
| untracked | `flow/tasks/T03-Host-Action-v1-纯逻辑实现.md` | project-flow-cy 记录 | 纯逻辑实现与测试证据 |
| untracked | `flow/tasks/T04-Host-Action-v1-八键配置覆盖.md` | project-flow-cy 记录 | 八键覆盖证据 |
| untracked | `flow/tasks/T06-Host-Action-v1-Vendor-HID-发送接入.md` | project-flow-cy 记录 | USB／BLE 单通道接入证据 |
| untracked | `flow/tasks/T08-Host-Action-v1-能力声明与BLE状态预算.md` | project-flow-cy 记录 | 能力与 512 字节状态预算证据 |
| untracked | `flow/tasks/T09-Host-Action-v1-软件侧总验收.md` | project-flow-cy 记录 | 59 项测试清单和 ESP-IDF 构建证据 |
| ignored | `build/` | 构建生成物 | 217 MiB ESP-IDF 生成目录，含 `.bin/.elf/.map`、CMake/Ninja 缓存和构建日志；由 `.gitignore` 的 `build/` 排除 |
| ignored | `build-host/` | 构建生成物 | 60 MiB 宿主 CMake／CTest 目录及测试二进制；由 `.gitignore` 的 `build-host/` 排除 |

分类统计：Host Action 生产实现 12 个、宿主测试 10 个、project-flow-cy 记录 10 个、忽略生成目录 2 个；课程开始前已有非 `flow/` 改动 0 个、无关改动 0 个、无法解释项 0 个。staged 文件为 0。

## 禁止修改与合同符合性矩阵

| # | 核对项 | 结果 | 文件／diff 证据 |
|---:|---|---|---|
| 1 | GPIO、`board_pins`、BOOT、GPIO8 共享电源 | PASS | `board_pins.h`、`gpio_keys.cpp`、`peripheral_power.cpp`、音频／LED／电池平台文件及硬件安全文档相对 `HEAD` 零差异。当前 v2 声明仍是 KEY1—KEY8=`2,47,38,41,1,6,7,48`，编码器=`17/16/18`，BOOT0=`0`，PWR_EN=`8` 高有效，USB=`19/20`，MIC=`9/10/11`，SPK=`14/13/15`，与 easyinput-board-cy 一致。扫描器 0 FAIL；两个 WARN 仅因未选择条件编译分支。 |
| 2 | Flash、分区、sdkconfig、目标和板型 | PASS | 根 `CMakeLists.txt`、`sdkconfig.defaults`、`partitions.csv` 相对基线零差异；默认板型仍仅允许 `v2`，目标仍为 `esp32s3`，Flash 配置仍为 16 MB，自定义分区仍为 `partitions.csv@0x8000`。被忽略的现有构建元数据也显示 `EASY_INPUT_BOARD=v2`、`IDF_TARGET=esp32s3`。 |
| 3 | USB／BLE 设备身份、描述符、GATT 与地址策略 | PASS | USB Report Descriptor 前后 SHA-1 同为 `97f0fe2b…`，USB 身份／字符串片段同为 `b8aa74c0…`；BLE Report Map 同为 `be8ba4bb…`，BLE UUID/GATT 定义同为 `543fb69b…`，两段 BLE 地址策略分别同为 `db832ec8…`、`d91a40ff…`。值仍为 VID/PID `0x303A/0x1006`、制造商 `AIOTWAN`、产品／BLE 名 `EasyInput AI`、USB serial `easy-input-v2`、BLE serial 空串、GATT revision 4；没有地址策略 diff。 |
| 4 | Host Action v1 冻结合同 | PASS | 共享头／实现固定 Report ID `0x11`、kind `0x05`、chunk `0`、total `1`、UUID len `36`、payload `63`；`status_hid_protocol.h` 仍固定状态 kind `0x04`。编码从完整配置值移除 12 字节前缀，只复制 36 字节 UUID；USB／BLE 都调用 `encode_host_action_v1()`。`transport_routing.cpp` 仍返回 `UsbFirst`，`app_main.cpp` 在 USB epoch 存在时无论成功失败均 `return`，仅无 USB owner 时调用 BLE。 |
| 5 | 固件不保存应用元数据，示例 UUID 仅在测试 | PASS | 生产范围检索没有应用路径、Bundle ID、应用名、图标或启动参数字段，也没有固定 UUID→应用映射。三个固定 UUID 的精确命中全部位于 `host_test/`；排除 `host_test/` 与 `flow/` 后，生产源码中的规范 `host_action:<36字符>` 固定值检索无输出。默认 Keymap 仍为既有 PTT、Return、Backspace、复制、粘贴、撤销和编码器动作。 |
| 6 | 没有新增隐私日志 | PASS | `git diff -U0 HEAD -- components main` 中没有新增 `ESP_LOG*` 行。`app_main.cpp` 只给既有分发日志增加固定 `kind="host_action"`，日志仍只包含序号、实体输入来源、kind、路由和连接布尔状态，不输出 `event.value`、UUID、应用信息或设备唯一标识。 |
| 7 | 复制、粘贴、快捷键、固定文字、音频、电源等无关功能 | PASS | 没有复制／粘贴、HID keycode、固定文字、音频 I/O 或 GPIO8 生命周期实现文件差异。`keymap.*` 只追加 Host Action 枚举／分支；`config_status.cpp`、`ble_status_wire.h` 与 BLE 状态调用变化均能逐项对应 T08 已批准的能力字段、speaker probe 预算、current power／cycle 中间回退和最终 512 字节检查，没有无法解释的业务变化。上一节点全量 59/59 是已有回归证据，本节点未重跑。 |
| 8 | `build/`、`build-host/` 与本机路径不进入提交范围 | PASS | `git check-ignore -v` 分别命中 `.gitignore` 的 `build/` 与 `build-host/`；`git ls-files build build-host` 无输出。排除生成目录后，对 Unix／Windows 本机绝对路径和 `file://` 的全仓检索无输出；二进制、映射、缓存和含本机路径的生成元数据都只在忽略目录中。 |

## diff 质量与写入边界

- 写入审计记录前，`git diff --check` 与 `git diff --cached --check` 均退出 0。
- 写入前对全部生产／测试变化文件及 `sdkconfig.defaults`、`partitions.csv` 计算的组合 SHA-1 为 `fc42199c51078b8e9bd7a088f5d46682fc01cee3`；写入 `flow/` 后必须再次得到同一值。
- 本节点不删除、不回退、不覆盖任何已有差异，也不执行测试、构建、设备识别、串口、App 或烧录。

## 仍未知与下一步

- 本审计只能证明从可靠 Git 基线到当前工作区的静态修改范围没有越过禁止边界；不能证明目标实物身份、USB／BLE 真机传输、运行时 GATT 状态、App 0.1.26 行为或烧录后启动。
- 如果下一节点要烧录，仍须单独获得烧录授权，并在执行前按项目规则识别和确认目标设备；本节点没有提供该授权，也没有访问设备。
