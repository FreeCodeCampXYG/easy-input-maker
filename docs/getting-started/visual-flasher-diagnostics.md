# 可视化烧录器诊断流程

本文给 EasyInput V2.0 / ESP32-S3 的可视化烧录工具定义最小可审计流程。它不替代用户确认，也不把端口名、烧录写入或正常启动混为同一结论。

## 状态机

| 阶段 | 工具动作 | UI 应显示的证据 | 失败时保留什么 |
| --- | --- | --- | --- |
| `scan` | 枚举当前串口候选 | 当前候选数、可读设备描述 | 不选择“唯一端口”以外的设备 |
| `identify` | 只读读取芯片、MAC、Flash 容量 | 芯片必须为 ESP32-S3、Flash 必须为 16 MB、MAC 仅显示尾号 | 身份读不到时不开放写入 |
| `verify_artifact` | 读取 manifest 或构建产物，计算 SHA-256 | 板型、IDF、三段镜像偏移和哈希匹配 | 哈希不匹配时保留下载文件，不写 Flash |
| `await_confirm` | 展示即将写入的设备与镜像摘要 | 端口、ESP32-S3、MAC 尾号、版本/提交或本地构建标识 | 取消后不发送写命令 |
| `flash` | 按固定偏移写 bootloader、分区表和应用 | 每段进度、esptool 写入校验结果 | 记录可脱敏错误码，禁止自动整片擦除 |
| `prepare_recovery` | 最后一次写前身份仍与确认目标一致 | 写入设备与确认设备的 MAC 一致 | 身份不一致时停止，不复用旧端口 |
| `normal_boot` | 按本轮下载方式恢复正常启动 | 自动复位或“需要完全关机再开机”提示 | 不重复要求按 BOOT |
| `observe` | 检查正常模式枚举，提示用户验证业务功能 | HID 枚举、用户观察的音符/旋钮结果 | 未观察到业务效果时标记待验证，不误报写入失败 |

## 镜像合同

当前构建的 `build/flasher_args.json` 是偏移的唯一来源。可视化工具应读取它或同等的 release manifest，而不是在 UI 中硬编码地址。

| 文件 | 偏移 | 写入后需要的证据 |
| --- | --- | --- |
| `bootloader/bootloader.bin` | `0x0` | esptool 写入与校验成功 |
| `partition_table/partition-table.bin` | `0x8000` | esptool 写入与校验成功 |
| `easy_input_keyboard.bin` | `0x10000` | esptool 写入与校验成功 |

写入前必须同时核对：目标为 `esp32s3`、Flash 为 16 MB、镜像的 SHA-256 与清单一致。普通烧录不执行 `erase-flash`。

## 下载模式与恢复

先尝试工具的自动复位。只有连接失败且尚未进入下载模式时，才提示用户在**开机状态短按并松开一次 BOOT**，等待重新枚举后重新识别芯片和 MAC。

若本轮手动使用过 BOOT，写入完成后应提示用户通过开发板电源开关完全关机再开机；正常启动时不再按 BOOT。工具不能把串口重新出现、写入命令退出 0 或 ROM 下载提示当作应用运行成功。

## 推荐的 UI 事件字段

日志和诊断页只保留最小字段，避免写入完整 MAC、原始串口输出、用户网络信息或本机绝对路径：

```json
{
  "phase": "identify",
  "result": "passed",
  "chip": "esp32s3",
  "flashMb": 16,
  "deviceMacSuffix": "xx:xx",
  "artifact": "local-build-or-release-tag",
  "messageCode": "IDENTITY_MATCHED"
}
```

`phase`、`result` 和稳定 `messageCode` 是诊断主键；中文说明仅供界面展示。遇到写入失败、设备断连或恢复失败时，保留前一阶段的通过证据和当前错误码，不能把整个过程笼统显示为“烧录失败”。

## 验证层级

1. `flash_verified`：三段镜像已写入并通过工具校验。
2. `normal_mode_detected`：设备已离开下载模式，枚举为预期 EasyInput USB/HID 集合。
3. `feature_observed`：使用者实际演奏八个音符、调节音量，并观察到预期效果。

前一层不推出后一层。当前离线钢琴首轮实板验收至少覆盖八个音符、三键和弦、5%/100% 音量边界、麦克风抢占及原有扬声器资产播放恢复。
