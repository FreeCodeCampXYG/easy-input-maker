#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "keyboard/music_synth.h"

namespace ai_keyboard {

enum class DrumBeatPattern : std::uint8_t {
  BeatFour,
  BeatThree,
  BeatSixEight,
  BeatTwo,
};

// 鼓机节拍只保存离线时钟和模式；平台层在唯一 PCM 所有者内取出命中的鼓声。
class DrumSequencer {
 public:
  void cycle_beat_mode();
  void toggle_sequential();
  void toggle_pause();
  void stop();
  void adjust_bpm(int delta_bpm);
  std::optional<DrumVoice> advance(std::size_t sample_count);

  DrumBeatPattern pattern() const;
  bool paused() const;
  bool session_active() const;
  bool sequential_enabled() const;
  std::uint16_t bpm() const;

 private:
  void reset_clock();
  std::uint32_t frames_until_next_ = 0;
  std::uint32_t frame_remainder_ = 0;
  std::uint8_t step_index_ = 0;
  DrumBeatPattern pattern_ = DrumBeatPattern::BeatFour;
  bool beat_enabled_ = false;
  bool sequential_enabled_ = false;
  std::uint16_t bpm_ = 120;
  bool paused_ = false;
};

}  // namespace ai_keyboard
