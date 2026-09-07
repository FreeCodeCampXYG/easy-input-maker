#include <array>
#include <cassert>

#include "keyboard/mobile_companion_protocol.h"
#include "keyboard/mobile_companion_session.h"

namespace {
using namespace ai_keyboard;
constexpr MobileCompanionConnection kConnection{7, 1};

MobileCompanionRequest mirror_request(MobileCompanionCommand command = MobileCompanionCommand::RequestMirror) {
  MobileCompanionRequest request;
  request.command = command;
  request.request_id = 42;
  request.capability = MobileCompanionCapability::Mirror;
  request.lease_ms = 1000;
  return request;
}

void fixed_wire_has_magic_and_crc() {
  std::array<std::uint8_t, kMobileCompanionFrameLen> frame{};
  assert(encode_mobile_companion_request(mirror_request(), &frame));
  MobileCompanionRequest decoded;
  assert(decode_mobile_companion_request(frame.data(), frame.size(), &decoded));
  assert(decoded.request_id == 42 && decoded.lease_ms == 1000);
  frame[14] ^= 0x01;
  assert(!decode_mobile_companion_request(frame.data(), frame.size(), &decoded));
}

void session_requires_encrypted_bonded_service_ready_and_expires() {
  MobileCompanionSession session;
  assert(session.handle(kConnection, mirror_request(), false, true, true, 0).result == MobileCompanionResult::NotEncrypted);
  assert(session.handle(kConnection, mirror_request(), true, false, true, 0).result == MobileCompanionResult::NotBonded);
  assert(session.handle(kConnection, mirror_request(), true, true, false, 0).result == MobileCompanionResult::InvalidFrame);
  assert(session.handle(kConnection, mirror_request(), true, true, true, 0).result == MobileCompanionResult::Accepted);
  assert(session.mirror_active(kConnection, 999));
  assert(session.expire(1000));
}

void duplicate_request_is_idempotent_and_generation_isolation_holds() {
  MobileCompanionSession session;
  const auto first = session.handle(kConnection, mirror_request(), true, true, true, 0);
  const auto duplicate = session.handle(kConnection, mirror_request(), true, true, true, 1);
  assert(first.generation == duplicate.generation && duplicate.result == MobileCompanionResult::Accepted);
  assert(!session.mirror_active({7, 2}, 1));
  assert(session.disconnect(kConnection));
  assert(!session.mirror_active(kConnection, 1));
}
}  // namespace

int main() {
  fixed_wire_has_magic_and_crc();
  session_requires_encrypted_bonded_service_ready_and_expires();
  duplicate_request_is_idempotent_and_generation_isolation_holds();
  return 0;
}
