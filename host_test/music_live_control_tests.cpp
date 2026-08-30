#include <cassert>

#include "keyboard/music_live_control.h"

namespace {

void eight_keys_map_to_the_piano_octave() {
  for (std::size_t index = 0; index < ai_keyboard::kMusicKeyCount; ++index) {
    const auto input = static_cast<ai_keyboard::InputId>(
        static_cast<std::size_t>(ai_keyboard::InputId::Key1) + index);
    assert(ai_keyboard::music_key_index_for_input(input) == index);
  }
  assert(!ai_keyboard::music_key_index_for_input(
      ai_keyboard::InputId::EncoderPress));
}

void volume_is_bounded_for_large_encoder_steps() {
  assert(ai_keyboard::adjusted_music_volume_percent(65, 5) == 70);
  assert(ai_keyboard::adjusted_music_volume_percent(5, -100) == 5);
  assert(ai_keyboard::adjusted_music_volume_percent(100, 100) == 100);
}

void final_key_state_preserves_a_dropped_release() {
  auto pressed = ai_keyboard::updated_music_pressed_mask(0, 3, true);
  assert(pressed == 0x08U);
  // 即使 release 没有进入边沿队列，最终状态仍明确要求音频任务释放该键。
  pressed = ai_keyboard::updated_music_pressed_mask(pressed, 3, false);
  assert(pressed == 0U);
  assert(ai_keyboard::music_pressed_mask_mismatch(0x08U, pressed) == 0x08U);
  assert(ai_keyboard::music_pressed_mask_mismatch(pressed, pressed) == 0U);
  assert(ai_keyboard::updated_music_pressed_mask(pressed, 8, true) == 0U);
}

}  // namespace

int main() {
  eight_keys_map_to_the_piano_octave();
  volume_is_bounded_for_large_encoder_steps();
  final_key_state_preserves_a_dropped_release();
  return 0;
}
