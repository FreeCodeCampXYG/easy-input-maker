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

}  // namespace

int main() {
  eight_keys_map_to_the_piano_octave();
  volume_is_bounded_for_large_encoder_steps();
  return 0;
}
