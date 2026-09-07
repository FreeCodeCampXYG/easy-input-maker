#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "keyboard/keymap.h"

namespace ai_keyboard {

// Mobile Companion v1 is an independent BLE GATT protocol. It deliberately
// does not reuse HID Vendor reports, AppCommand, or HostAction payloads.
inline constexpr std::uint8_t kMobileCompanionProtocolVersion = 1;
inline constexpr std::size_t kMobileCompanionFrameLen = 16;
inline constexpr std::uint32_t kMobileCompanionDefaultLeaseMs = 15000;
inline constexpr std::uint32_t kMobileCompanionMaxLeaseMs = 30000;

enum class MobileCompanionCapability : std::uint8_t {
  Unsupported = 0,
  Mirror = 1,
  Exclusive = 2,
};

enum class MobileCompanionCommand : std::uint8_t {
  Request = 1,
  Renew = 2,
  Release = 3,
};

enum class MobileCompanionResult : std::uint8_t {
  Accepted = 0,
  UnsupportedVersion = 1,
  InvalidFrame = 2,
  Unauthorized = 3,
  SessionExpired = 4,
  StaleConnection = 5,
  Busy = 6,
};

struct MobileCompanionConnection {
  static constexpr std::uint16_t kNoConnection = 0xFFFF;

  std::uint16_t conn_handle = kNoConnection;
  std::uint32_t generation = 0;

  bool valid() const { return conn_handle != kNoConnection && generation != 0; }
  bool operator==(const MobileCompanionConnection& other) const {
    return conn_handle == other.conn_handle && generation == other.generation;
  }
  bool operator!=(const MobileCompanionConnection& other) const {
    return !(*this == other);
  }
};

struct MobileCompanionRequest {
  MobileCompanionCommand command = MobileCompanionCommand::Request;
  MobileCompanionCapability capability = MobileCompanionCapability::Unsupported;
  std::uint32_t request_id = 0;
  std::uint32_t lease_ms = 0;
  // This bit is intentionally separate from BLE pairing. Pairing proves the
  // peer identity; Exclusive additionally requires an explicit v1 request.
  bool explicit_exclusive_authorization = false;
};

struct MobileCompanionAck {
  std::uint32_t request_id = 0;
  MobileCompanionResult result = MobileCompanionResult::InvalidFrame;
  MobileCompanionCapability granted = MobileCompanionCapability::Unsupported;
  std::uint32_t lease_ms = 0;
  std::uint32_t session_generation = 0;
};

struct MobileCompanionInputEvent {
  InputId input = InputId::Count;
  InputPhase phase = InputPhase::Pressed;
  std::int32_t encoder_step = 0;
  std::uint32_t input_sequence = 0;
};

bool decode_mobile_companion_request(const std::uint8_t* data,
                                     std::size_t len,
                                     MobileCompanionRequest* out);
bool encode_mobile_companion_ack(const MobileCompanionAck& ack,
                                 std::array<std::uint8_t, kMobileCompanionFrameLen>* out);
bool encode_mobile_companion_input_event(
    const MobileCompanionInputEvent& event,
    std::uint32_t session_generation,
    std::array<std::uint8_t, kMobileCompanionFrameLen>* out);

// Pure lease and connection-generation state. The platform authenticates a
// GATT write before it calls request(); this class still fail-closes every
// reconnect, timeout and owner mismatch so stale packets cannot be replayed.
class MobileCompanionSession {
 public:
  MobileCompanionAck request(const MobileCompanionConnection& connection,
                             const MobileCompanionRequest& request,
                             bool connection_authorized,
                             bool exclusive_transition_safe,
                             std::uint32_t now_ms);
  bool release(const MobileCompanionConnection& connection,
               std::uint32_t request_id,
               std::uint32_t now_ms);
  bool expire(std::uint32_t now_ms);
  bool disconnect(const MobileCompanionConnection& connection);
  bool accepts(const MobileCompanionConnection& connection,
               std::uint32_t now_ms) const;
  bool exclusive_active(const MobileCompanionConnection& connection,
                        std::uint32_t now_ms) const;
  MobileCompanionCapability capability() const { return capability_; }
  std::uint32_t session_generation() const { return session_generation_; }

 private:
  void clear();
  static bool time_reached(std::uint32_t now_ms, std::uint32_t deadline_ms);

  MobileCompanionConnection connection_{};
  MobileCompanionCapability capability_ = MobileCompanionCapability::Unsupported;
  std::uint32_t deadline_ms_ = 0;
  std::uint32_t session_generation_ = 0;
};

}  // namespace ai_keyboard
