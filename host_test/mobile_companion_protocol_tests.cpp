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

void default_keymap_contract_remains_explicit() {
  const auto keymap = DefaultKeymap();
  assert(keymap.action_for(InputId::Key1).kind == ActionKind::VoicePttHold);
  assert(keymap.action_for(InputId::Key3).kind == ActionKind::EditPttHold);
  assert(keymap.action_for(InputId::Key8).kind == ActionKind::Undo);
}

void mirror_allows_eight_keys_and_preserves_special_flags() {
  for (std::size_t index = 0; index < 8; ++index) {
    const auto input = static_cast<InputId>(index);
    assert(mobile_companion_mirror_input_allowed(input));
    const auto flags = mobile_companion_input_flags(input);
    if (input == InputId::Key1) {
      assert(flags == kMobileFlagKey1Voice);
    } else if (input == InputId::Key3) {
      assert(flags == kMobileFlagKey3Rewrite);
    } else if (input == InputId::Key8) {
      assert(flags == kMobileFlagKey8Shortcut);
    } else {
      assert(flags == 0);
    }
  }
  assert(!mobile_companion_mirror_input_allowed(InputId::EncoderLeft));
  assert(!mobile_companion_mirror_input_allowed(InputId::EncoderPress));
}

void encoded_events_keep_input_phase_identity_and_sequence() {
  for (std::size_t index = 0; index < 8; ++index) {
    for (const auto phase : {InputPhase::Pressed, InputPhase::Released}) {
      const auto input = static_cast<InputId>(index);
      const auto sequence = static_cast<std::uint32_t>(100 + index * 2 +
                                                        (phase == InputPhase::Released ? 1 : 0));
      MobileCompanionInputEvent event{};
      event.event_id = static_cast<std::uint16_t>(sequence);
      event.input = input;
      event.phase = phase;
      event.flags = mobile_companion_input_flags(input);
      event.generation = 0x10203040;
      event.sequence = sequence;
      std::array<std::uint8_t, kMobileCompanionFrameLen> frame{};
      assert(encode_mobile_companion_input_event(event, &frame));
      assert(frame[2] == static_cast<std::uint8_t>(MobileCompanionFrameType::Event));
      assert(frame[4] == static_cast<std::uint8_t>(sequence));
      assert(frame[5] == static_cast<std::uint8_t>(sequence >> 8));
      assert(frame[7] == static_cast<std::uint8_t>(
          event.flags | (phase == InputPhase::Pressed ? 1U : 2U) |
          (static_cast<std::uint8_t>(input) << 2)));
      assert(frame[8] == 0x40 && frame[9] == 0x30 && frame[10] == 0x20 &&
             frame[11] == 0x10);
      assert(frame[12] == static_cast<std::uint8_t>(sequence));
      assert(frame[13] == static_cast<std::uint8_t>(sequence >> 8));
      assert(mobile_companion_crc16(frame.data(), 14) ==
             static_cast<std::uint16_t>(frame[14] | (frame[15] << 8)));
    }
  }
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
  default_keymap_contract_remains_explicit();
  mirror_allows_eight_keys_and_preserves_special_flags();
  encoded_events_keep_input_phase_identity_and_sequence();
  return 0;
}
