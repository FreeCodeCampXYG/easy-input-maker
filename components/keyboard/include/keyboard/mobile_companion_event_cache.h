#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "keyboard/mobile_companion_protocol.h"

namespace ai_keyboard {

inline constexpr std::size_t kMobileCompanionEventCacheCapacity = 32;

enum class MobileCompanionReplayResult : std::uint8_t {
  Available, Empty, StaleGeneration, SequenceGap, WindowOverflow,
};

struct MobileCompanionReplayWindow {
  std::uint32_t generation = 0;
  std::uint32_t first_sequence = 0;
  std::uint32_t last_sequence = 0;
  std::uint32_t dropped_count = 0;
};

class MobileCompanionEventCache {
 public:
  MobileCompanionReplayResult push(const MobileCompanionInputEvent& event);
  MobileCompanionReplayResult query(std::uint32_t generation,
                                    std::uint32_t after_sequence,
                                    MobileCompanionReplayWindow* out) const;
  std::size_t copy_after(std::uint32_t generation,
                         std::uint32_t after_sequence,
                         std::array<MobileCompanionInputEvent,
                                    kMobileCompanionEventCacheCapacity>* out) const;
  void clear();
  std::uint32_t dropped_count() const { return dropped_count_; }
  std::size_t size() const { return count_; }

 private:
  std::array<MobileCompanionInputEvent, kMobileCompanionEventCacheCapacity> events_{};
  std::size_t head_ = 0;
  std::size_t count_ = 0;
  std::uint32_t generation_ = 0;
  std::uint32_t next_sequence_ = 0;
  std::uint32_t dropped_count_ = 0;
};

}  // namespace ai_keyboard
