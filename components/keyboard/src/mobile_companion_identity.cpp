#include "keyboard/mobile_companion_identity.h"

#include <cctype>

namespace ai_keyboard {

bool mobile_companion_device_id_valid(const std::string& value) {
  if (value.size() != kMobileCompanionDeviceIdLen) return false;
  for (const auto ch : value) {
    if (std::isdigit(static_cast<unsigned char>(ch)) == 0 &&
        (ch < 'A' || ch > 'Z')) return false;
  }
  return true;
}

}  // namespace ai_keyboard
