#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "keyboard/mobile_companion_protocol.h"

namespace ai_keyboard {

enum class MobileCompanionConfigResult : std::uint8_t {
  Pending, Complete, Invalid, OutOfOrder, Duplicate, CrcMismatch, TooLarge,
};

struct MobileCompanionConfigReceiveResult {
  MobileCompanionConfigResult result = MobileCompanionConfigResult::Invalid;
  std::string json;
};

class MobileCompanionConfigAssembler {
 public:
  MobileCompanionConfigReceiveResult receive(const MobileCompanionConfigFragment& fragment,
                                             std::uint32_t now_ms = 0);
  void reset();
  std::uint16_t request_id() const { return request_id_; }
  bool active() const { return active_; }

 private:
  std::array<std::uint8_t, kMobileCompanionConfigMaxBytes> buffer_{};
  std::uint16_t request_id_ = 0;
  std::uint16_t total_chunks_ = 0;
  std::uint16_t total_len_ = 0;
  std::uint16_t expected_crc_ = 0;
  std::uint16_t next_chunk_ = 0;
  std::uint16_t received_len_ = 0;
  bool active_ = false;
  std::uint32_t last_activity_ms_ = 0;
  std::uint16_t last_completed_request_id_ = 0;
  std::string last_completed_json_;
};

}  // namespace ai_keyboard
