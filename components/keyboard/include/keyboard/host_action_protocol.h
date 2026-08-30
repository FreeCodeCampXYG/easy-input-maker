#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ai_keyboard {

inline constexpr std::string_view kHostActionPrefix = "host_action:";
inline constexpr std::uint8_t kHostActionCommandKind = 0x05;
inline constexpr std::uint8_t kHostActionChunkIndex = 0;
inline constexpr std::uint8_t kHostActionTotalChunks = 1;
inline constexpr std::uint8_t kHostActionDataLen = 36;
inline constexpr std::size_t kHostActionPayloadLen = 63;

bool is_canonical_host_action(std::string_view value);

// 仅在输入完整通过校验后写入 payload，失败时保留调用方原缓冲区，避免产生半成品消息。
bool encode_host_action_app_command(
    std::string_view value,
    std::array<std::uint8_t, kHostActionPayloadLen>* payload);

}  // namespace ai_keyboard
