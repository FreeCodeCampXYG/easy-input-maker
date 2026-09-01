#pragma once

#include <cstdint>

namespace ai_keyboard {

enum class EncoderPressGesturePhase : std::uint8_t {
  Idle,
  Pending,
  FunctionCycleReady,
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

  // Transitions Pending -> FunctionCycleReady when the physical switch stays
  // down through the function-cycle confirmation deadline.
  bool trigger_function_cycle_if_due(std::uint32_t now_ms,
                                     std::uint32_t hold_ms,
                                     bool physically_pressed);

  // Transitions Pending/FunctionCycleReady -> ConfigTriggered exactly once
  // when the physical switch remains pressed through the configuration deadline.
  bool trigger_config_if_due(std::uint32_t now_ms,
                             std::uint32_t hold_ms,
                             bool physically_pressed);

  // 松开只区分短按、已确认功能切换和已进入配置模式；阈值在按住时确认，
  // 因此用户能根据灯光提示决定何时松开。
  EncoderPressReleaseResult release();
  void reset();

  EncoderPressGesturePhase phase() const;
  bool pending() const;
  bool config_triggered() const;
  bool function_cycle_ready() const;
  bool function_cycle_deadline(std::uint32_t hold_ms,
                               std::uint32_t* deadline_ms) const;
  bool config_deadline(std::uint32_t hold_ms,
                       std::uint32_t* deadline_ms) const;

 private:
  EncoderPressGesturePhase phase_ = EncoderPressGesturePhase::Idle;
  std::uint32_t down_ms_ = 0;
};

}  // namespace ai_keyboard
