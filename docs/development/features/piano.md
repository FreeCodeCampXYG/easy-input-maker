# L1 piano：八键钢琴（当前槽位 2）

入口：[总览](../README.md) · [共享合同](../shared.md)。稳定文档 ID 为 `piano`。

适用构建：`EASY_INPUT_SPEAKER_DIAGNOSTIC` 或 `EASY_INPUT_SPEAKER_ASSETS_PRODUCT`；关闭两者时没有此槽位。

## 功能与边界

八键音符、音阶/音色/力度、按键尾音与旋钮音量；当前最大 8 声部。是否可发声还受音乐配置控制，不等同于“切进槽位一定有声”。MUS1 参数、SEQ1 曲目和合成器是共享能力，rhythm 的内置曲目也使用它们。

## 读代码

1. `main/app_main.cpp`：dispatch_music_key、dispatch_music_volume、apply_music_config。
2. `components/keyboard/src/music_live_control.cpp`：键号/位图/音量；`components/keyboard/src/music_synth.cpp`：音高、包络、混音。
3. `components/keyboard/src/music_config_protocol.cpp`、`components/keyboard/src/music_sequence_player.cpp`：配置和曲目。
4. `main/platform/speaker_output.cpp`：request_music、queue_music_note、enqueue_music_command、consume_music_commands、play_music_frames。

## 怎样改

音高/音色算法优先改纯逻辑与测试，不在按键回调里操作 I2S。保持同键去重、8 键复音、满队列 release 补偿和固定混音余量。只做性能优化时要对照 PCM；有意改音色才在解释差异后更新指纹，禁止为让测试变绿直接换常量。

先运行：`ctest --test-dir build-host -R "music_|drum_|audio_io_arbiter|speaker_|firmware_source_contract" --output-on-failure`，再完整宿主回归。

实板：和弦、交叠按键、尾音、切槽、原版提示音/录音抢占、鼓机 K8。算法通过不代表音量或功耗已测量。
