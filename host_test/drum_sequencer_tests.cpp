#include <cassert>

#include "keyboard/drum_sequencer.h"

namespace {

void beat_key_cycles_conventional_meters() {
  ai_keyboard::DrumSequencer sequencer;
  sequencer.cycle_beat_mode();
  assert(sequencer.pattern() == ai_keyboard::DrumBeatPattern::BeatFour);
  sequencer.cycle_beat_mode();
  assert(sequencer.pattern() == ai_keyboard::DrumBeatPattern::BeatThree);
  sequencer.cycle_beat_mode();
  assert(sequencer.pattern() == ai_keyboard::DrumBeatPattern::BeatSixEight);
  sequencer.cycle_beat_mode();
  assert(sequencer.pattern() == ai_keyboard::DrumBeatPattern::BeatTwo);
  sequencer.cycle_beat_mode();
  assert(sequencer.pattern() == ai_keyboard::DrumBeatPattern::BeatFour);
  assert(sequencer.session_active());
  assert(sequencer.advance(0) == ai_keyboard::DrumVoice::Kick);
}

void six_eight_uses_eighth_note_steps() {
  ai_keyboard::DrumSequencer sequencer;
  sequencer.cycle_beat_mode();
  sequencer.cycle_beat_mode();
  sequencer.cycle_beat_mode();
  assert(sequencer.pattern() == ai_keyboard::DrumBeatPattern::BeatSixEight);
  assert(sequencer.advance(0) == ai_keyboard::DrumVoice::Kick);
  assert(sequencer.advance(12000) == ai_keyboard::DrumVoice::HiHat);
}

void sequential_mode_repeats_all_four_drum_voices() {
  ai_keyboard::DrumSequencer sequencer;
  sequencer.toggle_sequential();
  assert(sequencer.sequential_enabled());
  assert(sequencer.advance(0) == ai_keyboard::DrumVoice::Kick);
  assert(!sequencer.advance(1000).has_value());
  assert(sequencer.advance(24000) == ai_keyboard::DrumVoice::Snare);
  assert(sequencer.advance(24000) == ai_keyboard::DrumVoice::HiHat);
  assert(sequencer.advance(24000) == ai_keyboard::DrumVoice::Click);
  assert(sequencer.advance(24000) == ai_keyboard::DrumVoice::Kick);
}

void pause_and_tempo_are_bounded() {
  ai_keyboard::DrumSequencer sequencer;
  sequencer.toggle_sequential();
  assert(sequencer.sequential_enabled());
  sequencer.toggle_pause();
  assert(sequencer.paused());
  assert(!sequencer.advance(48000).has_value());
  sequencer.toggle_pause();
  assert(sequencer.advance(0) == ai_keyboard::DrumVoice::Kick);
  sequencer.adjust_bpm(1000);
  assert(sequencer.bpm() == 300);
  sequencer.adjust_bpm(-1000);
  assert(sequencer.bpm() == 20);
}

void frame_partition_does_not_accumulate_tempo_drift() {
  // 相同音频时长不能因 worker 分帧大小不同而累计丢拍。
  ai_keyboard::DrumSequencer single, blocked;
  single.adjust_bpm(17);
  blocked.adjust_bpm(17);
  single.toggle_sequential();
  blocked.toggle_sequential();
  int single_ticks = single.advance(0).has_value();
  int blocked_ticks = blocked.advance(0).has_value();
  for (std::size_t frame = 0; frame < 48000U * 60U; ++frame) {
    single_ticks += single.advance(1).has_value();
  }
  for (std::size_t frame = 0; frame < 48000U * 60U; frame += 480) {
    blocked_ticks += blocked.advance(480).has_value();
  }
  assert(single_ticks == 138);
  assert(blocked_ticks == single_ticks);
}

}  // namespace

int main() {
  frame_partition_does_not_accumulate_tempo_drift();
  beat_key_cycles_conventional_meters();
  six_eight_uses_eighth_note_steps();
  sequential_mode_repeats_all_four_drum_voices();
  pause_and_tempo_are_bounded();
  return 0;
}
