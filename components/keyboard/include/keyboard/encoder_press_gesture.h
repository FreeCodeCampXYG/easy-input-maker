#pragma once

#include <cstdint>

namespace ai_keyboard {

enum class EncoderPressGesturePhase : std::uint8_t {
  Idle,
  Pending,
  ConfigTriggered,
};

struct EncoderPressReleaseResult {
  bool dispatch_click = false;
  bool dispatch_function_cycle = false;
  bool open_config_mode = false;
  bool ignored_after_config = false;
};

// Classifies one physical encoder-press lifetime without knowing anything
// about USB, BLE or the target application. A normal release remains a click;
// only the existing three-second hold consumes it for system configuration.
class EncoderPressGesture {
 public:
  void press(std::uint32_t now_ms);

  // Transitions Pending -> ConfigTriggered exactly once when the physical
  // switch remains pressed through the hold deadline.
  bool trigger_config_if_due(std::uint32_t now_ms,
                             std::uint32_t hold_ms,
                             bool physically_pressed);

  // 在松开时按实际按住时长分类，避免中长按抢走原有短按动作。
  EncoderPressReleaseResult release(std::uint32_t now_ms,
                                    std::uint32_t function_cycle_hold_ms,
                                    std::uint32_t config_hold_ms);
  void reset();

  EncoderPressGesturePhase phase() const;
  bool pending() const;
  bool config_triggered() const;
  bool config_deadline(std::uint32_t hold_ms,
                       std::uint32_t* deadline_ms) const;

 private:
  EncoderPressGesturePhase phase_ = EncoderPressGesturePhase::Idle;
  std::uint32_t down_ms_ = 0;
};

}  // namespace ai_keyboard
