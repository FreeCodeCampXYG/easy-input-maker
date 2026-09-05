# L2 共享底座：改动前反查消费者

先读 [总览](README.md)。共享不等于可以随便改：这是三个功能交叉影响最集中的地方。

| 共享面 | 源码入口 | 必须复核的功能与失败边界 |
| --- | --- | --- |
| 输入与切槽 | `main/app_main.cpp`：handle_input_event、cycle_encoder_function；`encoder_press_gesture` | 全部功能；按住键切槽、切入失败保留旧槽、旋钮失败不回落 |
| HID 状态与传输 | `held_keyboard_state`、`keyboard_snapshot_delivery`、`transport_routing`；USB/BLE 适配 | keyboard + 所有切槽；6KRO、松键、掉线、epoch、旧 Host Action 不混入 AppCommand |
| 合成/曲目/音乐队列 | `music_synth`、`music_sequence_player`、`main/platform/speaker_output.cpp` | piano + rhythm + 原版声音资源/录音抢占；FIFO、满队列松键补偿、暂停/停止、尾音、播放状态 |
| 音频资源 | `audio_io_arbiter`、`speaker_playback`、`keyboard_audio`、`peripheral_power` | 全部功能；麦克风优先、generation 所有权、I2S 排空后再归还租约 |
| 设置与存储 | `config_receiver`、`config_payload`、`config_state`、MUS1/SEQ1、`nvs_store` | 全部功能；先验证再应用、保存与执行结果分开、旧配置可用、断电后行为明确 |
| 灯光/睡眠 | `led_strip_status`、`power_policy`、`awake_wait_planner` | 全部功能；不能把“灯灭”当成可以关闭 GPIO8，活跃任务与唤醒检查保持 |

上表没有扩展名的名称对应 `components/keyboard/src/<名称>.cpp`；`keyboard_audio`、`peripheral_power`、`nvs_store`、`led_strip_status` 位于 `main/platform/`。

## 验证范围

- 任何代码改动：先目标测试，再完整 `ctest --test-dir build-host --output-on-failure`。
- 修改音频共享代码：加测 piano/rhythm 组合和录音抢占，不能只听一个音。
- 修改输入/传输：加测 KEY1—KEY8、旋钮、长按切槽、USB/BLE 掉线重连、队列压力。
- 修改配置：测试旧配置、非法值、保存失败与 ACK；不改 MUS1/SEQ1 报告编号/容量，除非单独授权迁移。
- 修改平台层：必须有完整固件构建证据；目标语法检查仅是提前发现编译错误的辅助手段。
- GPIO/电源/睡眠需要 [硬件合同](../hardware/easyinput-v2-safety.md) 和实板验证。暂停保持 I2S 的实际功耗仍未测量。

## 当前尚未消除的耦合

原版声音资源不是第四个用户槽位。修改出厂提示音、资源同步/存储时，从 `features/speaker_assets/speaker_assets_runtime.cpp`、`speaker_assets_protocol.cpp`、`sound_asset_store.cpp` 和 `main/platform/speaker_assets_supervisor.cpp` 进入，联读 `speaker_output.cpp` 的资产请求/排空路径；验证 `ctest --test-dir build-host -R "speaker_assets|sound_asset|speaker_|firmware_source_contract" --output-on-failure` 后再完整回归。不要因 L1 只有三张槽位卡漏掉这个共享服务。

`app_main.cpp` 集中输入、切槽和配置应用；`music_mode_enabled` 与 `drum_mode_enabled` 同时决定输入归属；`SpeakerOutput` 内含钢琴、鼓机和序列状态。共享函数的修改可能影响两种甚至三种功能。新增文件夹和功能卡不改变这些事实。

未来拆分优先顺序：先为现有行为补组合测试，再抽功能输入处理，再引入单一活动功能状态，最后按确有需求增加注册信息。每步单独可审查，不把拆分、改协议和调声音合成一笔大改动。
