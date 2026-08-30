# T12 · Host Action v1 实践分支只读起点

> 会话：2026-08-30 · 分支 `my-host-action-practice`（cherry-pick 743368a 已解决冲突，未继续 git 操作）
> 范围：只读核对当前 Maker 固件；App 侧使用已准备好的 EasyInput 0.1.26。

## 本次目标

1. 用 project-flow-cy 记录本次目标与禁止越过的边界。
2. 只读梳理 `easy-input-maker` 工程分层。
3. 用 easyinput-board-cy 核对板型声明。
4. 用零基础语言交付：工程分层、按键链路入口、需保护的硬件与设备身份。

## 禁止越过的边界

- 不修改任何代码（本任务卡与 `flow/` 协作记录除外）。
- 不离开当前仓库 `d:/MyPro/easy-input-maker`。
- 不烧录、不引入 OTA/签名/部署工具。
- 不修改 GPIO、BOOT、分区、设备身份、通信协议、GPIO8 共享电源生命周期。
- 不提交真实凭据、Wi-Fi、BLE 地址、设备唯一标识、本机路径、原始串口日志。
- 本次只读核对，不实现任何 Host Action 功能；Host Action 缺位若出现在 starter 语义分支，是课程起点合同，不是待修回归。

## 只读核对结果（指针）

- 板型锁定：`CMakeLists.txt` 仅允许 `EASY_INPUT_BOARD=v2`（V2 已包含 V2.1 硬件修订）；`components/keyboard/CMakeLists.txt` 为 v2 定义 `EASY_INPUT_BOARD_V2=1`。
- `easyinput-board-cy` 的 `check_board_baseline.py`：1 PASS / 2 WARN / 0 FAIL；WARN（BUILD_BRANCH_UNSELECTED、PIN_DECLARATIONS_NOT_FOUND）来自静态扫描未带构建配置，GPIO 声明在条件编译内，非板型冲突。
- 设备身份：USB 序列号 `easy-input-v2`、制造商 `AIOTWAN`、产品 `EasyInput AI / EasyInput AI HID`、BLE 广播名 `EasyInput AI`、固件名 `EasyInput AI`（`board_pins.h`、`usb_hid.cpp`、`ble_hid.cpp`）。
- 分层与按键链路：详见本次会话回复（零基础语言说明）。

## 产出

- 本次会话回复中的零基础语言说明（本次不写代码、不写产品文档）。

## 下一步

- 按 `flow/plan.md` 从 T03 节点继续，每步先补宿主测试再实现；未经明确授权不烧录。
