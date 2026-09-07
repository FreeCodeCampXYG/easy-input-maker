#include <array>
#include <cassert>

#include "keyboard/mobile_companion_protocol.h"

namespace {

using ai_keyboard::MobileCompanionCapability;
using ai_keyboard::MobileCompanionCommand;
using ai_keyboard::MobileCompanionConnection;
using ai_keyboard::MobileCompanionRequest;
using ai_keyboard::MobileCompanionResult;
using ai_keyboard::MobileCompanionSession;

constexpr MobileCompanionConnection kPhoneA{7, 1};
constexpr MobileCompanionConnection kPhoneAReconnect{7, 2};

MobileCompanionRequest request(MobileCompanionCapability capability) {
  MobileCompanionRequest value;
  value.command = MobileCompanionCommand::Request;
  value.capability = capability;
  value.request_id = 42;
  value.lease_ms = 1000;
  value.explicit_exclusive_authorization = capability == MobileCompanionCapability::Exclusive;
  return value;
}

void mirror_never_claims_pc_input_and_expires_fail_closed() {
  MobileCompanionSession session;
  const auto ack = session.request(kPhoneA, request(MobileCompanionCapability::Mirror), true, true, 100);
  assert(ack.result == MobileCompanionResult::Accepted);
  assert(ack.granted == MobileCompanionCapability::Mirror);
  assert(session.accepts(kPhoneA, 1099));
  assert(!session.exclusive_active(kPhoneA, 1099));
  assert(session.expire(1100));
  assert(!session.accepts(kPhoneA, 1100));
}

void exclusive_requires_both_explicit_request_and_safe_transition() {
  MobileCompanionSession session;
  auto exclusive = request(MobileCompanionCapability::Exclusive);
  exclusive.explicit_exclusive_authorization = false;
  assert(session.request(kPhoneA, exclusive, true, true, 0).result ==
         MobileCompanionResult::Unauthorized);
  exclusive.explicit_exclusive_authorization = true;
  assert(session.request(kPhoneA, exclusive, true, false, 0).result ==
         MobileCompanionResult::Unauthorized);
  const auto ack = session.request(kPhoneA, exclusive, true, true, 0);
  assert(ack.result == MobileCompanionResult::Accepted);
  assert(session.exclusive_active(kPhoneA, 1));
}

void reconnect_generation_cannot_resume_or_release_old_session() {
  MobileCompanionSession session;
  assert(session.request(kPhoneA, request(MobileCompanionCapability::Mirror), true, true, 0).result ==
         MobileCompanionResult::Accepted);
  assert(!session.accepts(kPhoneAReconnect, 1));
  assert(!session.release(kPhoneAReconnect, 42, 1));
  auto other_request = request(MobileCompanionCapability::Mirror);
  assert(session.request(kPhoneAReconnect, other_request, true, true, 1).result ==
         MobileCompanionResult::Busy);
  assert(session.disconnect(kPhoneA));
  assert(!session.accepts(kPhoneA, 1));
}

void wire_is_versioned_and_fixed_size() {
  std::array<std::uint8_t, ai_keyboard::kMobileCompanionFrameLen> frame{};
  frame[0] = ai_keyboard::kMobileCompanionProtocolVersion;
  frame[1] = 1;
  frame[2] = static_cast<std::uint8_t>(MobileCompanionCommand::Request);
  frame[3] = static_cast<std::uint8_t>(MobileCompanionCapability::Mirror);
  frame[4] = 0x2A;
  frame[8] = 0xE8;
  frame[9] = 0x03;
  MobileCompanionRequest decoded;
  assert(ai_keyboard::decode_mobile_companion_request(frame.data(), frame.size(), &decoded));
  assert(decoded.request_id == 42 && decoded.lease_ms == 1000);
  frame[0] = 2;
  assert(!ai_keyboard::decode_mobile_companion_request(frame.data(), frame.size(), &decoded));
}

}  // namespace

int main() {
  mirror_never_claims_pc_input_and_expires_fail_closed();
  exclusive_requires_both_explicit_request_and_safe_transition();
  reconnect_generation_cannot_resume_or_release_old_session();
  wire_is_versioned_and_fixed_size();
  return 0;
}
