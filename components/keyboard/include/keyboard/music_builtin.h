#pragma once

#include "keyboard/music_sequence_protocol.h"

namespace ai_keyboard {

// 返回固件内置的公版测试曲目；未知编号返回空值，避免把任意值当作曲目执行。
std::optional<MusicSequence> builtin_music_sequence(std::uint8_t builtin_id);

}  // namespace ai_keyboard
