#include "keyboard/music_live_control.h"

#include <algorithm>

namespace ai_keyboard {

std::optional<std::size_t> music_key_index_for_input(InputId input) {
  const auto value = static_cast<std::size_t>(input);
  const auto first = static_cast<std::size_t>(InputId::Key1);
  const auto last = static_cast<std::size_t>(InputId::Key8);
  if (value < first || value > last) {
    return std::nullopt;
  }
  return value - first;
}

std::uint8_t adjusted_music_volume_percent(std::uint8_t current,
                                           int delta_percent) {
  return static_cast<std::uint8_t>(
      std::clamp(static_cast<int>(current) + delta_percent, 5, 100));
}

std::uint8_t updated_music_pressed_mask(std::uint8_t current,
                                        std::size_t key_index,
                                        bool pressed) {
  if (key_index >= kMusicKeyCount) {
    return current;
  }
  const auto bit = static_cast<std::uint8_t>(1U << key_index);
  return pressed ? static_cast<std::uint8_t>(current | bit)
                 : static_cast<std::uint8_t>(current & ~bit);
}

std::uint8_t music_pressed_mask_mismatch(std::uint8_t applied,
                                         std::uint8_t desired) {
  return static_cast<std::uint8_t>(applied ^ desired);
}

}  // namespace ai_keyboard
