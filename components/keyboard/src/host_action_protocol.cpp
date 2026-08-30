#include "keyboard/host_action_protocol.h"

#include <algorithm>

namespace ai_keyboard {
namespace {

bool is_lower_hex(char value) {
  return (value >= '0' && value <= '9') ||
         (value >= 'a' && value <= 'f');
}

}  // namespace

bool is_canonical_host_action(std::string_view value) {
  if (value.size() != kHostActionPrefix.size() + kHostActionDataLen ||
      value.substr(0, kHostActionPrefix.size()) != kHostActionPrefix) {
    return false;
  }

  const auto uuid = value.substr(kHostActionPrefix.size());
  for (std::size_t index = 0; index < uuid.size(); ++index) {
    const bool separator = index == 8 || index == 13 || index == 18 || index == 23;
    if (separator) {
      if (uuid[index] != '-') {
        return false;
      }
    } else if (!is_lower_hex(uuid[index])) {
      return false;
    }
  }
  return true;
}

bool encode_host_action_app_command(
    std::string_view value,
    std::array<std::uint8_t, kHostActionPayloadLen>* payload) {
  if (payload == nullptr || !is_canonical_host_action(value)) {
    return false;
  }

  std::array<std::uint8_t, kHostActionPayloadLen> encoded{};
  encoded[0] = kHostActionCommandKind;
  encoded[1] = kHostActionChunkIndex;
  encoded[2] = kHostActionTotalChunks;
  encoded[3] = kHostActionDataLen;
  const auto uuid = value.substr(kHostActionPrefix.size());
  std::copy_n(uuid.begin(), uuid.size(), encoded.begin() + 4);
  *payload = encoded;
  return true;
}

}  // namespace ai_keyboard
