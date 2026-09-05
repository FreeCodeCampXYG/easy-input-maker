# L1 rhythm：节拍器与鼓机（当前槽位 3）

入口：[总览](../README.md) · [共享合同](../shared.md)。稳定文档 ID 为 `rhythm`。

适用构建：`EASY_INPUT_SPEAKER_DIAGNOSTIC` 或 `EASY_INPUT_SPEAKER_ASSETS_PRODUCT`；关闭两者时没有此槽位。

## 功能与边界

K1—K4 触发鼓声，K5 循环 4/4、3/4、6/8、2/4；K7 控制顺序鼓声；K6 在 K8 曲目活跃时停止曲目，否则暂停/恢复鼓机。K8 播放内置《欢乐颂》。旋钮平时调速，曲目活跃时调音量；该行为以当前实现为准。

MUS1 的 click 节拍器是另一个共用合成器功能，不要把它与 DrumSequencer 拍号状态混为同一个开关。BPM 采用四分音符口径，6/8 每步为八分音符。

## 读代码

1. `main/app_main.cpp`：dispatch_music_key 中的 drum 分支、dispatch_music_volume。
2. `components/keyboard/src/drum_sequencer.cpp`：拍号、顺序、暂停与帧时钟。
3. `components/keyboard/src/music_synth.cpp`：鼓声/click；`components/keyboard/src/music_builtin.cpp` 和 `components/keyboard/src/music_sequence_player.cpp`：内置曲目与播放。
4. `main/platform/speaker_output.cpp`：音乐命令队列、worker 消费顺序、状态发布。

## 怎样改

改拍号/时值先做整分钟跨帧时钟测试；不要直接用主循环延时打拍。共享 voice 池必须区分琴键和鼓声。调速失败仍由 rhythm 消费输入，不得落回原版滚动。暂停只停声音/时钟不意味着已释放硬件或达到低功耗。

先运行：`ctest --test-dir build-host -R "drum_|music_|audio_io_arbiter|speaker_|firmware_source_contract" --output-on-failure`，再完整宿主回归。

已知缺口：内置长曲目不适合外部 27 事件 SEQ1 存储格式；启动读取曲目尚未形成恢复播放能力。不得为了本功能把外部报告容量改大。

实板：各拍号、长期节拍、暂停/恢复、K6/K8、切回钢琴/键盘、快速控制与录音抢占。
