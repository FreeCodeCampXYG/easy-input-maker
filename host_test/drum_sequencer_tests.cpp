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

}  // namespace

int main() {
  beat_key_cycles_conventional_meters();
  six_eight_uses_eighth_note_steps();
  sequential_mode_repeats_all_four_drum_voices();
  pause_and_tempo_are_bounded();
  return 0;
}
