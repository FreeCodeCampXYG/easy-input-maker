#pragma once

#include <cstddef>

namespace ai_keyboard {

// 纯逻辑环形索引：只在已有功能槽位中前进，末尾后回到首槽位。
class EncoderFunctionRing {
 public:
  explicit EncoderFunctionRing(std::size_t slot_count = 1U)
      : slot_count_(slot_count) {}

  std::size_t current() const { return current_; }
  std::size_t size() const { return slot_count_; }

  // 空环保持当前索引；非空环按一次短按前进一个槽位。
  std::size_t advance() {
    if (slot_count_ == 0U) {
      return current_;
    }
    current_ = (current_ + 1U) % slot_count_;
    return current_;
  }

  void set_current(std::size_t slot) {
    current_ = slot_count_ == 0U ? 0U : slot % slot_count_;
  }

 private:
  std::size_t slot_count_ = 1U;
  std::size_t current_ = 0U;
};

}  // namespace ai_keyboard
