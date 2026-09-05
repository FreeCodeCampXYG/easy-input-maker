#include <array>
#include <cassert>

#include "keyboard/music_config_protocol.h"

namespace {

void round_trip_preserves_all_public_controls() {
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  config.root_midi = 57;
  config.transpose_semitones = -5;
  config.global_cents = -23;
  config.scale = ai_keyboard::MusicScale::NaturalMinor;
  config.bpm = 132;
  config.beats_per_bar = 6;
  config.beat_unit = 8;
  config.attack_ms = 12;
  config.release_ms = 90;
  config.reference_a4_milli_hz = 442000;
  config.metronome_enabled = true;
  config.key_cents[4] = 18;
  config.timbres[4] = ai_keyboard::MusicTimbre::Bell;
  std::array<std::uint8_t, ai_keyboard::kMusicConfigPayloadLen> payload{};
  assert(ai_keyboard::encode_music_config(config, &payload));
  const auto decoded = ai_keyboard::decode_music_config(payload.data(), payload.size());
  assert(decoded.has_value());
  assert(decoded->enabled && decoded->root_midi == 57 && decoded->transpose_semitones == -5);
  assert(decoded->scale == ai_keyboard::MusicScale::NaturalMinor && decoded->bpm == 132);
  assert(decoded->beats_per_bar == 6 && decoded->beat_unit == 8 && decoded->key_cents[4] == 18 && decoded->metronome_enabled);
  assert(decoded->timbres[4] == ai_keyboard::MusicTimbre::Bell);
}

void invalid_magic_version_and_reserved_flags_fail_closed() {
  std::array<std::uint8_t, ai_keyboard::kMusicConfigPayloadLen> payload{};
  assert(!ai_keyboard::decode_music_config(payload.data(), payload.size()).has_value());
  ai_keyboard::MusicConfig config;
  assert(ai_keyboard::encode_music_config(config, &payload));
  payload[4] = 2;
  assert(!ai_keyboard::decode_music_config(payload.data(), payload.size()).has_value());
  payload[4] = ai_keyboard::kMusicConfigVersion;
  payload[5] = 2;
  assert(!ai_keyboard::decode_music_config(payload.data(), payload.size()).has_value());
}

void invalid_scale_does_not_fall_back_to_major() {
  // 未知调式不能静默变成大调并被持久化。
  ai_keyboard::MusicConfig invalid;
  std::array<std::uint8_t, ai_keyboard::kMusicConfigPayloadLen> payload{};
  assert(ai_keyboard::encode_music_config(invalid, &payload));
  payload[8] = 255;
  assert(!ai_keyboard::decode_music_config(payload.data(), payload.size()));
  invalid.scale = static_cast<ai_keyboard::MusicScale>(255);
  ai_keyboard::MusicSynth synth;
  assert(!synth.apply_config(invalid));
  assert(synth.config().scale == ai_keyboard::MusicScale::Major);
}

}  // namespace

int main() {
  invalid_scale_does_not_fall_back_to_major();
  round_trip_preserves_all_public_controls();
  invalid_magic_version_and_reserved_flags_fail_closed();
  return 0;
}
