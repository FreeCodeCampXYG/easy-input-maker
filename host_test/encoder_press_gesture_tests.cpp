#include <cassert>
#include <cstdint>

#include "keyboard/encoder_press_gesture.h"

namespace {

constexpr std::uint32_t kFunctionCycleHoldMs = 800U;
constexpr std::uint32_t kConfigHoldMs = 3000U;

void short_press_remains_a_click() {
  ai_keyboard::EncoderPressGesture gesture;
  gesture.press(10U);
  const auto released = gesture.release(799U, kFunctionCycleHoldMs, kConfigHoldMs);
  assert(released.dispatch_click);
}

void medium_hold_cycles_functions_on_release() {
  ai_keyboard::EncoderPressGesture gesture;
  gesture.press(100U);
  const auto released = gesture.release(900U, kFunctionCycleHoldMs, kConfigHoldMs);
  assert(!released.dispatch_click);
  assert(released.dispatch_function_cycle);
  assert(!released.open_config_mode);
}

void no_rotation_preserves_the_three_second_config_entry() {
  ai_keyboard::EncoderPressGesture gesture;
  gesture.press(0xFFFFFF00U);
  assert(!gesture.trigger_config_if_due(0x00000050U, 1000U, true));
  assert(gesture.trigger_config_if_due(0x00000320U, 1000U, true));
  assert(gesture.config_triggered());

  const auto released = gesture.release(0x00000320U, kFunctionCycleHoldMs, 1000U);
  assert(!released.dispatch_click);
  assert(released.ignored_after_config);
}

void sampled_time_has_an_exact_three_second_boundary() {
  ai_keyboard::EncoderPressGesture before_deadline;
  before_deadline.press(100U);
  assert(!before_deadline.trigger_config_if_due(3099U, 3000U, true));
  assert(before_deadline.release(3099U, kFunctionCycleHoldMs, 3000U)
             .dispatch_function_cycle);

  ai_keyboard::EncoderPressGesture at_deadline;
  at_deadline.press(100U);
  assert(at_deadline.trigger_config_if_due(3100U, 3000U, true));

  ai_keyboard::EncoderPressGesture after_deadline;
  after_deadline.press(100U);
  assert(after_deadline.trigger_config_if_due(3101U, 3000U, true));

  ai_keyboard::EncoderPressGesture release_at_deadline;
  release_at_deadline.press(100U);
  assert(release_at_deadline
             .release(3100U, kFunctionCycleHoldMs, 3000U)
             .open_config_mode);
}

}  // namespace

int main() {
  short_press_remains_a_click();
  medium_hold_cycles_functions_on_release();
  no_rotation_preserves_the_three_second_config_entry();
  sampled_time_has_an_exact_three_second_boundary();
  return 0;
}
