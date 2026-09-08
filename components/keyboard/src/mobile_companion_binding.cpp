#include "keyboard/mobile_companion_binding.h"

namespace ai_keyboard {

bool mobile_binding_action_valid(const Action& action) {
  if (action.kind == ActionKind::FixedText &&
      action.text.size() > kMobileBindingFixedTextMaxLen) {
    return false;
  }
  return action.kind != ActionKind::Disabled ||
         (action.hotkey.empty() && action.text.empty());
}

bool MobileBindingOverlay::set(InputId input, const Action& action) {
  const auto index = static_cast<std::size_t>(input);
  if (index >= actions_.size() || !mobile_binding_action_valid(action)) {
    return false;
  }
  actions_[index] = action;
  present_[index] = true;
  return true;
}

void MobileBindingOverlay::clear(InputId input) {
  const auto index = static_cast<std::size_t>(input);
  if (index < present_.size()) {
    present_[index] = false;
    actions_[index] = {};
  }
}

void MobileBindingOverlay::clear_all() {
  present_.fill(false);
  actions_ = {};
}

bool MobileBindingOverlay::has(InputId input) const {
  const auto index = static_cast<std::size_t>(input);
  return index < present_.size() && present_[index];
}

const Action& MobileBindingOverlay::action_for(InputId input) const {
  static const Action kDisabled{};
  const auto index = static_cast<std::size_t>(input);
  return index < actions_.size() && present_[index] ? actions_[index] : kDisabled;
}

}  // namespace ai_keyboard
