#include <cassert>

#include "keyboard/mobile_companion_identity.h"

int main() {
  using namespace ai_keyboard;
  assert(mobile_companion_device_id_valid("A1B2C3D4"));
  assert(!mobile_companion_device_id_valid("a1B2C3D4"));
  assert(!mobile_companion_device_id_valid("A1B2"));
  return 0;
}
