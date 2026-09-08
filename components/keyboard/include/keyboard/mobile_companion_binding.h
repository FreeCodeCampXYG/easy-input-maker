#pragma once

#include <array>
#include <cstddef>
#include <string>

#include "keyboard/keymap.h"

namespace ai_keyboard {

inline constexpr std::size_t kMobileBindingFixedTextMaxLen = 512;

// Mobile bindings are session-local overlays. They never mutate the persisted
// device Keymap, so disconnecting the phone restores the PC configuration.
class MobileBindingOverlay {
 public:
  bool set(InputId input, const Action& action);
  void clear(InputId input);
  void clear_all();
  bool has(InputId input) const;
  const Action& action_for(InputId input) const;

 private:
  std::array<Action, static_cast<std::size_t>(InputId::Count)> actions_{};
  std::array<bool, static_cast<std::size_t>(InputId::Count)> present_{};
};

bool mobile_binding_action_valid(const Action& action);

}  // namespace ai_keyboard
