#include "keyboard/mobile_companion_protocol.h"

#include <algorithm>

namespace ai_keyboard {
namespace {

constexpr std::uint8_t kFrameKindCommand = 0x01;
constexpr std::uint8_t kFrameKindAck = 0x02;
constexpr std::uint8_t kFrameKindInput = 0x03;
constexpr std::uint8_t kExclusiveAuthorizedFlag = 0x01;

std::uint32_t read_u32_le(const std::uint8_t* data) {
  return static_cast<std::uint32_t>(data[0]) |
         (static_cast<std::uint32_t>(data[1]) << 8) |
         (static_cast<std::uint32_t>(data[2]) << 16) |
         (static_cast<std::uint32_t>(data[3]) << 24);
}

void write_u32_le(std::uint32_t value, std::uint8_t* out) {
  out[0] = static_cast<std::uint8_t>(value);
  out[1] = static_cast<std::uint8_t>(value >> 8);
  out[2] = static_cast<std::uint8_t>(value >> 16);
  out[3] = static_cast<std::uint8_t>(value >> 24);
}

bool valid_capability(std::uint8_t value) {
  return value <= static_cast<std::uint8_t>(MobileCompanionCapability::Exclusive);
}

bool valid_command(std::uint8_t value) {
  return value >= static_cast<std::uint8_t>(MobileCompanionCommand::Request) &&
         value <= static_cast<std::uint8_t>(MobileCompanionCommand::Release);
}

}  // namespace

bool decode_mobile_companion_request(const std::uint8_t* data,
                                     std::size_t len,
                                     MobileCompanionRequest* out) {
  if (data == nullptr || out == nullptr || len != kMobileCompanionFrameLen ||
      data[0] != kMobileCompanionProtocolVersion || data[1] != kFrameKindCommand ||
      !valid_command(data[2]) || !valid_capability(data[3])) {
    return false;
  }
  out->command = static_cast<MobileCompanionCommand>(data[2]);
  out->capability = static_cast<MobileCompanionCapability>(data[3]);
  out->request_id = read_u32_le(data + 4);
  out->lease_ms = read_u32_le(data + 8);
  out->explicit_exclusive_authorization = (data[12] & kExclusiveAuthorizedFlag) != 0;
  return true;
}

bool encode_mobile_companion_ack(const MobileCompanionAck& ack,
                                 std::array<std::uint8_t, kMobileCompanionFrameLen>* out) {
  if (out == nullptr) {
    return false;
  }
  out->fill(0);
  (*out)[0] = kMobileCompanionProtocolVersion;
  (*out)[1] = kFrameKindAck;
  (*out)[2] = static_cast<std::uint8_t>(ack.result);
  (*out)[3] = static_cast<std::uint8_t>(ack.granted);
  write_u32_le(ack.request_id, out->data() + 4);
  write_u32_le(ack.lease_ms, out->data() + 8);
  write_u32_le(ack.session_generation, out->data() + 12);
  return true;
}

bool encode_mobile_companion_input_event(
    const MobileCompanionInputEvent& event,
    std::uint32_t session_generation,
    std::array<std::uint8_t, kMobileCompanionFrameLen>* out) {
  if (out == nullptr || event.input >= InputId::Count) {
    return false;
  }
  out->fill(0);
  (*out)[0] = kMobileCompanionProtocolVersion;
  (*out)[1] = kFrameKindInput;
  (*out)[2] = static_cast<std::uint8_t>(event.input);
  (*out)[3] = event.phase == InputPhase::Pressed ? 1 : 2;
  write_u32_le(event.input_sequence, out->data() + 4);
  write_u32_le(session_generation, out->data() + 8);
  write_u32_le(static_cast<std::uint32_t>(event.encoder_step), out->data() + 12);
  return true;
}

MobileCompanionAck MobileCompanionSession::request(
    const MobileCompanionConnection& connection,
    const MobileCompanionRequest& request,
    bool connection_authorized,
    bool exclusive_transition_safe,
    std::uint32_t now_ms) {
  MobileCompanionAck ack;
  ack.request_id = request.request_id;
  if (!connection.valid() || !connection_authorized) {
    ack.result = MobileCompanionResult::Unauthorized;
    return ack;
  }
  if (request.capability == MobileCompanionCapability::Unsupported) {
    ack.result = MobileCompanionResult::InvalidFrame;
    return ack;
  }
  if (request.command == MobileCompanionCommand::Release) {
    if (!accepts(connection, now_ms)) {
      ack.result = MobileCompanionResult::StaleConnection;
      return ack;
    }
    clear();
    ack.result = MobileCompanionResult::Accepted;
    return ack;
  }
  const bool active = capability_ != MobileCompanionCapability::Unsupported &&
                      !time_reached(now_ms, deadline_ms_);
  if (active && connection_ != connection) {
    ack.result = MobileCompanionResult::Busy;
    return ack;
  }
  if (request.command == MobileCompanionCommand::Renew &&
      (!accepts(connection, now_ms) || request.capability != capability_)) {
    ack.result = MobileCompanionResult::SessionExpired;
    return ack;
  }
  if (request.capability == MobileCompanionCapability::Exclusive &&
      (!request.explicit_exclusive_authorization || !exclusive_transition_safe)) {
    ack.result = MobileCompanionResult::Unauthorized;
    return ack;
  }
  const auto lease = std::min(
      request.lease_ms == 0 ? kMobileCompanionDefaultLeaseMs : request.lease_ms,
      kMobileCompanionMaxLeaseMs);
  if (!accepts(connection, now_ms) || capability_ != request.capability) {
    ++session_generation_;
    if (session_generation_ == 0) {
      ++session_generation_;
    }
  }
  connection_ = connection;
  capability_ = request.capability;
  deadline_ms_ = now_ms + lease;
  ack.result = MobileCompanionResult::Accepted;
  ack.granted = capability_;
  ack.lease_ms = lease;
  ack.session_generation = session_generation_;
  return ack;
}

bool MobileCompanionSession::release(const MobileCompanionConnection& connection,
                                     std::uint32_t request_id,
                                     std::uint32_t now_ms) {
  (void)request_id;
  if (!accepts(connection, now_ms)) {
    return false;
  }
  clear();
  return true;
}

bool MobileCompanionSession::expire(std::uint32_t now_ms) {
  if (capability_ == MobileCompanionCapability::Unsupported || !time_reached(now_ms, deadline_ms_)) {
    return false;
  }
  clear();
  return true;
}

bool MobileCompanionSession::disconnect(const MobileCompanionConnection& connection) {
  if (connection_ != connection) {
    return false;
  }
  clear();
  return true;
}

bool MobileCompanionSession::accepts(const MobileCompanionConnection& connection,
                                     std::uint32_t now_ms) const {
  return capability_ != MobileCompanionCapability::Unsupported &&
         connection_ == connection && !time_reached(now_ms, deadline_ms_);
}

bool MobileCompanionSession::exclusive_active(const MobileCompanionConnection& connection,
                                               std::uint32_t now_ms) const {
  return accepts(connection, now_ms) && capability_ == MobileCompanionCapability::Exclusive;
}

void MobileCompanionSession::clear() {
  connection_ = {};
  capability_ = MobileCompanionCapability::Unsupported;
  deadline_ms_ = 0;
}

bool MobileCompanionSession::time_reached(std::uint32_t now_ms, std::uint32_t deadline_ms) {
  return static_cast<std::int32_t>(now_ms - deadline_ms) >= 0;
}

}  // namespace ai_keyboard
