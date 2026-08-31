#include "keyboard/music_sequence_protocol.h"

namespace ai_keyboard {

bool encode_music_sequence(const MusicSequence& sequence,
                           std::array<std::uint8_t, kMusicSequencePayloadLen>* payload) {
  if (payload == nullptr || sequence.event_count > kMusicSequenceMaxEvents ||
      sequence.bpm < 20 || sequence.bpm > 300 ||
      (sequence.command != MusicSequenceCommand::Stop &&
       sequence.command != MusicSequenceCommand::Play &&
       sequence.command != MusicSequenceCommand::Builtin)) {
    return false;
  }
  if (sequence.command == MusicSequenceCommand::Builtin &&
      (sequence.event_count != 0 || sequence.builtin_id == 0)) {
    return false;
  }
  payload->fill(0);
  (*payload)[0] = 'S'; (*payload)[1] = 'E'; (*payload)[2] = 'Q'; (*payload)[3] = '1';
  (*payload)[4] = kMusicSequenceVersion;
  (*payload)[5] = static_cast<std::uint8_t>(sequence.command);
  (*payload)[6] = static_cast<std::uint8_t>(sequence.bpm);
  (*payload)[7] = static_cast<std::uint8_t>(sequence.bpm >> 8U);
  (*payload)[8] = sequence.event_count;
  (*payload)[9] = sequence.builtin_id;
  if (sequence.command == MusicSequenceCommand::Builtin) return true;
  for (std::size_t index = 0; index < sequence.event_count; ++index) {
    const auto& event = sequence.events[index];
    if ((event.degree != kMusicSequenceRest && event.degree > 7) ||
        event.duration_sixteenths == 0 || event.duration_sixteenths > 255 ||
        event.velocity_percent > 100) {
      return false;
    }
    // 高 4 位保存 0—15 档力度，低 3 位保存音阶级数；旧版 0—7
    // 编码仍解码为满音量，保持 Studio/旧固件报告兼容。
    const auto velocity_step = static_cast<std::uint8_t>(
        (static_cast<std::uint16_t>(event.velocity_percent) * 15U + 50U) / 100U);
    (*payload)[9 + index * 2U] = static_cast<std::uint8_t>(
        event.degree | static_cast<std::uint8_t>(velocity_step << 3U));
    (*payload)[10 + index * 2U] = event.duration_sixteenths;
  }
  return true;
}

std::optional<MusicSequence> decode_music_sequence(const std::uint8_t* payload,
                                                    std::size_t length) {
  if (payload == nullptr || length != kMusicSequencePayloadLen ||
      payload[0] != 'S' || payload[1] != 'E' || payload[2] != 'Q' ||
      payload[3] != '1' || payload[4] != kMusicSequenceVersion ||
      payload[5] > static_cast<std::uint8_t>(MusicSequenceCommand::Builtin) ||
      payload[8] > kMusicSequenceMaxEvents) {
    return std::nullopt;
  }
  MusicSequence sequence;
  sequence.command = static_cast<MusicSequenceCommand>(payload[5]);
  sequence.builtin_id = payload[9];
  sequence.bpm = static_cast<std::uint16_t>(payload[6]) |
                 (static_cast<std::uint16_t>(payload[7]) << 8U);
  sequence.event_count = payload[8];
  for (std::size_t index = 0; index < sequence.event_count; ++index) {
    const auto encoded_degree = payload[9 + index * 2U];
    const auto degree = static_cast<std::uint8_t>(encoded_degree & 0x07U);
    const auto velocity_step = static_cast<std::uint8_t>(encoded_degree >> 3U);
    const auto velocity_percent = static_cast<std::uint8_t>(
        encoded_degree == kMusicSequenceRest || velocity_step == 0U
            ? 100U
            : (velocity_step * 100U + 7U) / 15U);
    sequence.events[index] = {
        encoded_degree == kMusicSequenceRest ? kMusicSequenceRest : degree,
        payload[10 + index * 2U],
        velocity_percent};
  }
  if (sequence.command == MusicSequenceCommand::Builtin && sequence.builtin_id == 0) {
    return std::nullopt;
  }
  std::array<std::uint8_t, kMusicSequencePayloadLen> encoded{};
  return encode_music_sequence(sequence, &encoded) ? std::optional<MusicSequence>(sequence)
                                                   : std::nullopt;
}

}  // namespace ai_keyboard
