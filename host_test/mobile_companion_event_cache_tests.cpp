#include <cassert>

#include "keyboard/mobile_companion_event_cache.h"

int main() {
  using namespace ai_keyboard;
  MobileCompanionEventCache cache;
  for (std::uint32_t sequence = 1; sequence <= 40; ++sequence) {
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
