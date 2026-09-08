#include <cassert>
#include <string>

#include "keyboard/keymap.h"
#include "keyboard/mobile_companion_binding.h"

int main() {
  using namespace ai_keyboard;
  const auto defaults = DefaultKeymap();
  MobileBindingOverlay overlay;
  assert(!overlay.has(InputId::Key1));
  assert(overlay.set(InputId::Key1, {ActionKind::FixedText, "", "phone"}));
  assert(overlay.has(InputId::Key1));
  assert(overlay.action_for(InputId::Key1).text == "phone");
  assert(defaults.action_for(InputId::Key1).kind == ActionKind::VoicePttHold);
  assert(!overlay.set(InputId::Key3,
                      {ActionKind::FixedText, "", std::string(513, 'x')}));
  overlay.clear(InputId::Key1);
  assert(!overlay.has(InputId::Key1));
  return 0;
}
