#include <cassert>

#include "keyboard/drum_sequencer.h"

namespace {

void beat_key_cycles_four_three_and_off() {
  ai_keyboard::DrumSequencer sequencer;
  sequencer.cycle_beat_mode();
  assert(sequencer.mode() == ai_keyboard::DrumLoopMode::BeatFour);
  assert(sequencer.advance(0) == ai_keyboard::DrumVoice::Kick);
  sequencer.cycle_beat_mode();
  assert(sequencer.mode() == ai_keyboard::DrumLoopMode::BeatThree);
  sequencer.cycle_beat_mode();
  assert(!sequencer.session_active());
}

void sequential_mode_repeats_all_four_drum_voices() {
  ai_keyboard::DrumSequencer sequencer;
  sequencer.toggle_sequential();
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

}  // namespace

int main() {
  beat_key_cycles_four_three_and_off();
  sequential_mode_repeats_all_four_drum_voices();
  pause_and_tempo_are_bounded();
  return 0;
}
