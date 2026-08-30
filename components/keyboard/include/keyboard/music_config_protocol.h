#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "keyboard/music_synth.h"

namespace ai_keyboard {

constexpr std::uint8_t kMusicConfigReportId = 0x16;
constexpr std::size_t kMusicConfigPayloadLen = 63;
constexpr std::uint8_t kMusicConfigVersion = 1;

// Music Config v1 是独立于通用键盘 JSON 的固定 Feature Report，避免重同步删除音乐设置。
bool encode_music_config(const MusicConfig& config,
                         std::array<std::uint8_t, kMusicConfigPayloadLen>* payload);
std::optional<MusicConfig> decode_music_config(const std::uint8_t* payload,
                                               std::size_t length);

}  // namespace ai_keyboard
