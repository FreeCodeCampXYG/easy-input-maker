#pragma once

#include <cstddef>
#include <string>

namespace ai_keyboard {

inline constexpr std::size_t kMobileCompanionDeviceIdLen = 8;

bool mobile_companion_device_id_valid(const std::string& value);

}  // namespace ai_keyboard
