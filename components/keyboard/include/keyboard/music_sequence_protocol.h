#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ai_keyboard {

constexpr std::uint8_t kMusicSequenceReportId = 0x17;
constexpr std::uint8_t kMusicSequenceVersion = 1;
constexpr std::size_t kMusicSequencePayloadLen = 63;
constexpr std::size_t kMusicSequenceMaxEvents = 27;
// 内置曲目不受 63 字节外部 wire 容量限制，但仍共享同一播放器和事件格式。
constexpr std::size_t kMusicSequenceStorageEvents = 128;
constexpr std::uint8_t kMusicSequenceRest = 0xFF;

enum class MusicSequenceCommand : std::uint8_t {
  Stop = 0, Play = 1, Builtin = 2, Pause = 3, Resume = 4
};

constexpr std::uint8_t kBuiltinSongOdeToJoy = 1;

struct MusicSequenceEvent {
  std::uint8_t degree = kMusicSequenceRest;
  std::uint8_t duration_sixteenths = 4;
  // 0 表示沿用旧版 wire 的满音量；有效范围 1—100。
  std::uint8_t velocity_percent = 100;
};

struct MusicSequence {
  MusicSequenceCommand command = MusicSequenceCommand::Stop;
  std::uint8_t builtin_id = 0;
  std::uint16_t bpm = 120;
  std::array<MusicSequenceEvent, kMusicSequenceStorageEvents> events{};
  std::uint8_t event_count = 0;
};

bool encode_music_sequence(const MusicSequence& sequence,
                           std::array<std::uint8_t, kMusicSequencePayloadLen>* payload);
std::optional<MusicSequence> decode_music_sequence(const std::uint8_t* payload,
                                                    std::size_t length);

}  // namespace ai_keyboard
