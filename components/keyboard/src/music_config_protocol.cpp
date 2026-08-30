#include "keyboard/music_config_protocol.h"

namespace ai_keyboard {
namespace {
void put_u16(std::array<std::uint8_t, kMusicConfigPayloadLen>* out, std::size_t offset, std::uint16_t value) {
  (*out)[offset] = static_cast<std::uint8_t>(value);
  (*out)[offset + 1] = static_cast<std::uint8_t>(value >> 8U);
}
std::uint16_t get_u16(const std::uint8_t* value, std::size_t offset) {
  return static_cast<std::uint16_t>(value[offset]) | (static_cast<std::uint16_t>(value[offset + 1]) << 8U);
}
}

bool encode_music_config(const MusicConfig& config,
                         std::array<std::uint8_t, kMusicConfigPayloadLen>* payload) {
  if (payload == nullptr || !validate_music_config(config)) return false;
  payload->fill(0);
  (*payload)[0] = 'M'; (*payload)[1] = 'U'; (*payload)[2] = 'S'; (*payload)[3] = '1';
  (*payload)[4] = kMusicConfigVersion;
  (*payload)[5] = config.enabled ? 1U : 0U;
  (*payload)[6] = static_cast<std::uint8_t>(config.root_midi);
  (*payload)[7] = static_cast<std::uint8_t>(config.transpose_semitones);
  (*payload)[8] = static_cast<std::uint8_t>(config.scale);
  put_u16(payload, 9, static_cast<std::uint16_t>(config.global_cents));
  put_u16(payload, 11, config.bpm);
  (*payload)[13] = config.beats_per_bar; (*payload)[14] = config.beat_unit;
  put_u16(payload, 15, config.attack_ms); put_u16(payload, 17, config.release_ms);
  put_u16(payload, 19, static_cast<std::uint16_t>(config.reference_a4_milli_hz / 10U));
  for (std::size_t i = 0; i < kMusicKeyCount; ++i) {
    put_u16(payload, 21 + i * 2U, static_cast<std::uint16_t>(config.key_cents[i]));
    (*payload)[37 + i] = static_cast<std::uint8_t>(config.timbres[i]);
  }
  (*payload)[45] = config.metronome_enabled ? 1U : 0U;
  return true;
}

std::optional<MusicConfig> decode_music_config(const std::uint8_t* payload, std::size_t length) {
  if (payload == nullptr || length != kMusicConfigPayloadLen || payload[0] != 'M' || payload[1] != 'U' ||
      payload[2] != 'S' || payload[3] != '1' || payload[4] != kMusicConfigVersion || (payload[5] & ~1U) != 0 ||
      (payload[45] & ~1U) != 0) return std::nullopt;
  MusicConfig config;
  config.enabled = payload[5] != 0; config.root_midi = static_cast<std::int8_t>(payload[6]);
  config.transpose_semitones = static_cast<std::int8_t>(payload[7]); config.scale = static_cast<MusicScale>(payload[8]);
  config.global_cents = static_cast<std::int16_t>(get_u16(payload, 9)); config.bpm = get_u16(payload, 11);
  config.beats_per_bar = payload[13]; config.beat_unit = payload[14]; config.attack_ms = get_u16(payload, 15);
  config.release_ms = get_u16(payload, 17); config.reference_a4_milli_hz = static_cast<std::uint32_t>(get_u16(payload, 19)) * 10U;
  for (std::size_t i = 0; i < kMusicKeyCount; ++i) {
    config.key_cents[i] = static_cast<std::int16_t>(get_u16(payload, 21 + i * 2U));
    config.timbres[i] = static_cast<MusicTimbre>(payload[37 + i]);
  }
  config.metronome_enabled = payload[45] != 0;
  if (!validate_music_config(config)) return std::nullopt;
  return config;
}
}  // namespace ai_keyboard
