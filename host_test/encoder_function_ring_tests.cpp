#include <cassert>

#include "keyboard/encoder_function_ring.h"

void cycles_and_wraps_at_the_end() {
  ai_keyboard::EncoderFunctionRing ring(3U);
  assert(ring.current() == 0U);
  assert(ring.advance() == 1U);
  assert(ring.advance() == 2U);
  assert(ring.advance() == 0U);
}

void empty_ring_is_safe_and_single_slot_is_stable() {
  ai_keyboard::EncoderFunctionRing empty(0U);
  assert(empty.advance() == 0U);

  ai_keyboard::EncoderFunctionRing one(1U);
  assert(one.advance() == 0U);
  one.set_current(99U);
  assert(one.current() == 0U);
}

int main() {
  cycles_and_wraps_at_the_end();
  empty_ring_is_safe_and_single_slot_is_stable();
  return 0;
}
