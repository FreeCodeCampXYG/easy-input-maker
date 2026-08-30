#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

#include "keyboard/keymap.h"
#include "keyboard/music_synth.h"

namespace ai_keyboard {

// 将实体键与音符索引固定在纯逻辑层，平台只负责把边沿转交给唯一音频所有者。
std::optional<std::size_t> music_key_index_for_input(InputId input);
std::uint8_t adjusted_music_volume_percent(std::uint8_t current,
                                           int delta_percent);
std::uint8_t updated_music_pressed_mask(std::uint8_t current,
                                        std::size_t key_index,
                                        bool pressed);
std::uint8_t music_pressed_mask_mismatch(std::uint8_t applied,
                                         std::uint8_t desired);

}  // namespace ai_keyboard
