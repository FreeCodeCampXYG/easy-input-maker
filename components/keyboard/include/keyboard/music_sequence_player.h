#pragma once

#include <cstddef>
#include <cstdint>

#include "keyboard/music_sequence_protocol.h"
#include "keyboard/music_synth.h"

namespace ai_keyboard {

class MusicSequencePlayer {
 public:
  bool load(const MusicSequence& sequence);
  void stop(MusicSynth* synth);
  void advance(std::size_t sample_count, MusicSynth* synth);
  bool playing() const;

 private:
  MusicSequence sequence_{};
  std::uint16_t event_index_ = 0;
  std::uint32_t event_frames_remaining_ = 0;
  std::uint64_t frame_remainder_ = 0;
  bool playing_ = false;
};

}  // namespace ai_keyboard
