#include "keyboard/mobile_companion_event_cache.h"

namespace ai_keyboard {

MobileCompanionReplayResult MobileCompanionEventCache::push(
    const MobileCompanionInputEvent& event) {
  if (event.generation == 0 || event.sequence == 0) {
    return MobileCompanionReplayResult::StaleGeneration;
  }
  if (generation_ != event.generation) {
    clear();
    generation_ = event.generation;
  }
  if (next_sequence_ != 0 && event.sequence <= next_sequence_) {
    return MobileCompanionReplayResult::SequenceGap;
  }
  if (next_sequence_ != 0 && event.sequence != next_sequence_ + 1) {
    ++dropped_count_;
  }
  next_sequence_ = event.sequence;
  if (count_ == events_.size()) {
    events_[head_] = event;
    head_ = (head_ + 1) % events_.size();
    ++dropped_count_;
    return MobileCompanionReplayResult::WindowOverflow;
  }
  const auto index = (head_ + count_) % events_.size();
  events_[index] = event;
  ++count_;
  return MobileCompanionReplayResult::Available;
}

MobileCompanionReplayResult MobileCompanionEventCache::query(
    std::uint32_t generation,
    std::uint32_t after_sequence,
    MobileCompanionReplayWindow* out) const {
  if (out == nullptr || generation == 0 || generation != generation_) {
    return MobileCompanionReplayResult::StaleGeneration;
  }
  if (count_ == 0) {
    return MobileCompanionReplayResult::Empty;
  }
  const auto first = events_[head_].sequence;
  const auto last = events_[(head_ + count_ - 1) % events_.size()].sequence;
  out->generation = generation_;
  out->first_sequence = first;
  out->last_sequence = last;
  out->dropped_count = dropped_count_;
  if (after_sequence + 1 < first) {
    return MobileCompanionReplayResult::WindowOverflow;
  }
  if (after_sequence >= last) {
    return MobileCompanionReplayResult::Empty;
  }
  return MobileCompanionReplayResult::Available;
}

std::size_t MobileCompanionEventCache::copy_after(
    std::uint32_t generation,
    std::uint32_t after_sequence,
    std::array<MobileCompanionInputEvent,
               kMobileCompanionEventCacheCapacity>* out) const {
  if (out == nullptr || generation == 0 || generation != generation_) return 0;
  std::size_t copied = 0;
  for (std::size_t offset = 0; offset < count_ && copied < out->size(); ++offset) {
    const auto& event = events_[(head_ + offset) % events_.size()];
    if (event.sequence > after_sequence) (*out)[copied++] = event;
  }
  return copied;
}

void MobileCompanionEventCache::clear() {
  events_ = {};
  head_ = 0;
  count_ = 0;
  next_sequence_ = 0;
}

}  // namespace ai_keyboard
