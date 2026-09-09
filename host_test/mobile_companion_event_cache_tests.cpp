#include <cassert>

#include "keyboard/mobile_companion_event_cache.h"

int main() {
  using namespace ai_keyboard;
  MobileCompanionEventCache game_cache;
  std::uint32_t game_sequence = 1;
  for (std::size_t input_index = 0; input_index < 8; ++input_index) {
    for (const auto phase : {InputPhase::Pressed, InputPhase::Released}) {
      MobileCompanionInputEvent event{};
      event.event_id = static_cast<std::uint16_t>(game_sequence);
      event.input = static_cast<InputId>(input_index);
      event.phase = phase;
      event.generation = 9;
      event.sequence = game_sequence++;
      assert(game_cache.push(event) == MobileCompanionReplayResult::Available);
    }
  }
  MobileCompanionReplayWindow game_window;
  assert(game_cache.query(9, 0, &game_window) == MobileCompanionReplayResult::Available);
  std::array<MobileCompanionInputEvent, kMobileCompanionEventCacheCapacity> game_replay{};
  assert(game_cache.copy_after(9, 0, &game_replay) == 16);
  for (std::size_t index = 0; index < 8; ++index) {
    assert(game_replay[index * 2].input == static_cast<InputId>(index));
    assert(game_replay[index * 2].phase == InputPhase::Pressed);
    assert(game_replay[index * 2 + 1].input == static_cast<InputId>(index));
    assert(game_replay[index * 2 + 1].phase == InputPhase::Released);
  }

  MobileCompanionEventCache cache;
  std::uint32_t sequence = 1;
  for (std::size_t input_index = 0; input_index < 8; ++input_index) {
    for (const auto phase : {InputPhase::Pressed, InputPhase::Released}) {
      MobileCompanionInputEvent event{};
      event.event_id = static_cast<std::uint16_t>(sequence);
      event.input = static_cast<InputId>(input_index);
      event.phase = phase;
      event.generation = 7;
      event.sequence = sequence++;
      assert(cache.push(event) == MobileCompanionReplayResult::Available);
    }
  }
  for (; sequence <= 40; ++sequence) {
    MobileCompanionInputEvent event{};
    event.event_id = static_cast<std::uint16_t>(sequence);
    event.input = InputId::Key1;
    event.generation = 7;
    event.sequence = sequence;
    cache.push(event);
  }
  MobileCompanionReplayWindow window;
  assert(cache.size() == kMobileCompanionEventCacheCapacity);
  assert(cache.dropped_count() == 8);
  assert(cache.query(7, 1, &window) == MobileCompanionReplayResult::WindowOverflow);
  assert(window.first_sequence == 9 && window.last_sequence == 40);
  std::array<MobileCompanionInputEvent, kMobileCompanionEventCacheCapacity> replay{};
  assert(cache.copy_after(7, 38, &replay) == 2);
  assert(replay[0].sequence == 39 && replay[1].sequence == 40);
  assert(cache.query(6, 39, &window) == MobileCompanionReplayResult::StaleGeneration);
  return 0;
}
