#include <cassert>
#include <cmath>
#include <array>

#include "keyboard/music_synth.h"

namespace {

void envelope_cache_preserves_pcm_fingerprints() {
  // 优化前录下的 PCM 指纹：四种音色、八键复音、播放中改配置和长尾音。
  constexpr std::array<std::uint64_t, 8> expected{{
      5344675918021175315ULL, 9527039558125785837ULL,
      6235575889531046513ULL, 11861811407602363855ULL,
      3608587429469577061ULL, 686405771077223468ULL,
      8248977947322849955ULL, 4742436947417588223ULL}};
  for (unsigned scenario = 0; scenario < expected.size(); ++scenario) {
    ai_keyboard::MusicConfig config;
    config.enabled = true;
    config.attack_ms = scenario * 17;
    config.release_ms = scenario * 31;
    config.timbres.fill(static_cast<ai_keyboard::MusicTimbre>(scenario % 4));
    ai_keyboard::MusicSynth synth;
    assert(synth.apply_config(config));
    for (unsigned key = 0; key < 8; ++key) assert(synth.note_on(key, 30 + key * 10));
    std::array<std::int16_t, 480> pcm{};
    std::uint64_t hash = 14695981039346656037ULL;
    for (unsigned frame = 0; frame < 200; ++frame) {
      if (frame == 50) {
        config.attack_ms = 2000;
        config.release_ms = 4000;
        assert(synth.apply_config(config));
      }
      if (frame == 100) {
        for (unsigned key = 0; key < 8; ++key) assert(synth.note_off(key));
      }
      synth.render(pcm.data(), pcm.size());
      for (const auto sample : pcm) {
        hash ^= static_cast<std::uint16_t>(sample);
        hash *= 1099511628211ULL;
      }
    }
    assert(hash == expected[scenario]);
  }
}

void defaults_are_c_major_from_c4_to_c5() {
  ai_keyboard::MusicConfig config;
  assert(std::abs(ai_keyboard::music_key_frequency_hz(config, 0) - 261.625565) < 0.001);
  assert(std::abs(ai_keyboard::music_key_frequency_hz(config, 5) - 440.0) < 0.001);
  assert(std::abs(ai_keyboard::music_key_frequency_hz(config, 7) - 523.251131) < 0.001);
}

void octave_and_cents_follow_equal_temperament() {
  ai_keyboard::MusicConfig config;
  config.root_midi = 69;
  config.scale = ai_keyboard::MusicScale::Custom;
  config.custom_offsets.fill(0);
  assert(std::abs(ai_keyboard::music_key_frequency_hz(config, 0) - 440.0) < 0.001);
  config.transpose_semitones = 12;
  assert(std::abs(ai_keyboard::music_key_frequency_hz(config, 0) - 880.0) < 0.001);
  config.transpose_semitones = 0;
  config.global_cents = 100;
  assert(std::abs(ai_keyboard::music_key_frequency_hz(config, 0) - 466.163762) < 0.001);
}

void scale_selection_and_phase_increment_are_valid() {
  ai_keyboard::MusicConfig config;
  config.scale = ai_keyboard::MusicScale::NaturalMinor;
  assert(ai_keyboard::music_key_semitone(config, 2) == 3);
  config.scale = ai_keyboard::MusicScale::PentatonicMajor;
  assert(ai_keyboard::music_key_semitone(config, 7) == 16);
  assert(ai_keyboard::music_key_phase_increment(config, 0) != 0U);
}

void adsr_release_and_phase_are_continuous() {
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  config.attack_ms = 2;
  config.release_ms = 2;
  ai_keyboard::MusicSynth synth;
  assert(synth.apply_config(config));
  assert(synth.note_on(0));
  std::array<std::int16_t, 160> first{};
  synth.render(first.data(), first.size());
  assert(synth.note_on(0));
  std::array<std::int16_t, 2> retrigger{};
  synth.render(retrigger.data(), retrigger.size());
  assert(std::abs(static_cast<int>(retrigger[1]) - retrigger[0]) < 32000);
  assert(synth.note_off(0));
  std::array<std::int16_t, 200> released{};
  synth.render(released.data(), released.size());
  assert(!synth.active());
}

void metronome_and_tuner_are_offline_and_deterministic() {
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  config.metronome_enabled = true;
  config.bpm = 300;
  config.beats_per_bar = 3;
  config.beat_unit = 4;
  ai_keyboard::MusicSynth synth;
  assert(synth.apply_config(config));
  std::array<std::int16_t, 1> silent{};
  synth.render(silent.data(), silent.size());
  assert(silent[0] != 0);
  bool accented = false;
  assert(synth.take_metronome_tick(&accented));
  assert(accented);
  assert(synth.metronome_running());
  constexpr std::size_t sample_count = 4800;
  std::array<std::int16_t, sample_count> sine{};
  for (std::size_t i = 0; i < sine.size(); ++i) {
    sine[i] = static_cast<std::int16_t>(14000.0 * std::sin(2.0 * 3.141592653589793 * 440.0 * i / 48000.0));
  }
  const auto estimate = ai_keyboard::estimate_tuner_frequency(sine.data(), sine.size());
  assert(estimate.valid);
  assert(std::abs(estimate.frequency_hz - 440.0) < 3.0);
  assert(std::abs(estimate.cents_from_a4) < 15.0);
}

void three_voice_chord_keeps_headroom() {
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  config.attack_ms = 0;
  config.release_ms = 20;
  ai_keyboard::MusicSynth synth;
  assert(synth.apply_config(config));
  assert(synth.note_on(0));
  assert(synth.note_on(2));
  assert(synth.note_on(4));
  std::array<std::int16_t, 480> frame{};
  synth.render(frame.data(), frame.size());
  for (const auto sample : frame) {
    assert(sample < 32767 && sample > -32768);
  }
}

void all_overlapping_keys_keep_audible_voices() {
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  config.attack_ms = 0;
  config.release_ms = 45;
  ai_keyboard::MusicSynth synth;
  assert(synth.apply_config(config));
  for (std::size_t key = 0; key < ai_keyboard::kMusicKeyCount; ++key) {
    assert(synth.note_on(key));
  }
  std::array<std::int16_t, 480> frame{};
  synth.render(frame.data(), frame.size());
  bool audible = false;
  for (const auto sample : frame) {
    audible = audible || sample != 0;
  }
  assert(audible);
  assert(synth.active());
}

void drum_voices_render_and_decay() {
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  config.release_ms = 2;
  ai_keyboard::MusicSynth synth;
  assert(synth.apply_config(config));
  assert(synth.trigger_drum(ai_keyboard::DrumVoice::Kick));
  std::array<std::int16_t, 256> frame{};
  synth.render(frame.data(), frame.size());
  bool audible = false;
  for (const auto sample : frame) audible = audible || sample != 0;
  assert(audible);
  assert(!synth.active());
}

void six_eight_clicks_use_eighth_notes() {
  ai_keyboard::MusicConfig meter_config;
  meter_config.enabled = true;
  meter_config.metronome_enabled = true;
  meter_config.beats_per_bar = 6;
  meter_config.beat_unit = 8;
  ai_keyboard::MusicSynth meter;
  assert(meter.apply_config(meter_config));
  std::array<std::int16_t, 1> tick{};
  meter.render(tick.data(), 1);
  assert(meter.take_metronome_tick(nullptr));
  for (int frame = 0; frame < 12000; ++frame) meter.render(tick.data(), 1);
  assert(meter.take_metronome_tick(nullptr));
}

void piano_keys_do_not_reuse_percussion_voices() {
  // 钢琴松键不能释放或冒认仍在衰减的鼓声声部。
  ai_keyboard::MusicSynth mixed;
  ai_keyboard::MusicConfig mixed_config;
  mixed_config.enabled = true;
  assert(mixed.apply_config(mixed_config));
  assert(mixed.trigger_drum(ai_keyboard::DrumVoice::Kick));
  assert(!mixed.note_off(0));
  assert(mixed.note_on(0));
  assert(mixed.note_off(0));
  // 新音符必须占用独立 voice；分别渲染再相加与同池混音仅差整数舍入。
  ai_keyboard::MusicSynth drum_only, note_only;
  assert(drum_only.apply_config(mixed_config));
  assert(note_only.apply_config(mixed_config));
  assert(drum_only.trigger_drum(ai_keyboard::DrumVoice::Kick));
  assert(note_only.note_on(0));
  assert(note_only.note_off(0));
  std::array<std::int16_t, 128> combined{}, drums{}, notes{};
  mixed.render(combined.data(), combined.size());
  drum_only.render(drums.data(), drums.size());
  note_only.render(notes.data(), notes.size());
  for (std::size_t i = 0; i < combined.size(); ++i) {
    assert(std::abs(static_cast<int>(combined[i]) - drums[i] - notes[i]) <= 1);
  }
}

}  // namespace

int main() {
  envelope_cache_preserves_pcm_fingerprints();
  six_eight_clicks_use_eighth_notes();
  piano_keys_do_not_reuse_percussion_voices();
  defaults_are_c_major_from_c4_to_c5();
  octave_and_cents_follow_equal_temperament();
  scale_selection_and_phase_increment_are_valid();
  adsr_release_and_phase_are_continuous();
  metronome_and_tuner_are_offline_and_deterministic();
  three_voice_chord_keeps_headroom();
  all_overlapping_keys_keep_audible_voices();
  drum_voices_render_and_decay();
  return 0;
}
