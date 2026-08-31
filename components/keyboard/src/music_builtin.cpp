#include "keyboard/music_builtin.h"

namespace ai_keyboard {

std::optional<MusicSequence> builtin_music_sequence(std::uint8_t builtin_id) {
  if (builtin_id != kBuiltinSongOdeToJoy) return std::nullopt;

  // 《欢乐颂》主题旋律为公版测试素材；数组只保存音阶级数、时值和力度，
  // 播放时由 MusicSynth 根据当前根音换算频率，不存放 PCM 音频。
  constexpr std::uint8_t kMelody[] = {
      1, 1, 2, 3, 3, 2, 1, 0, 0, 1, 1, 2, 3,
      3, 2, 1, 0, 0, 1, 1, 2, 3, 2, 1, 0};
  constexpr std::uint8_t kVelocity[] = {
      90, 75, 80, 90, 85, 75, 90, 70, 70, 85, 75, 80, 90,
      85, 75, 90, 70, 70, 85, 75, 90, 80, 75, 90, 65};
  MusicSequence sequence;
  sequence.command = MusicSequenceCommand::Play;
  sequence.bpm = 100;
  sequence.event_count = sizeof(kMelody) / sizeof(kMelody[0]);
  for (std::size_t index = 0; index < sequence.event_count; ++index) {
    sequence.events[index] = {kMelody[index], 2, kVelocity[index]};
  }
  return sequence;
}

}  // namespace ai_keyboard
