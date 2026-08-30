#include <array>
#include <cassert>
#include <cstdint>
#include <string>

#include "keyboard/host_action_protocol.h"
#include "keyboard/config_payload.h"
#include "keyboard/keymap.h"
#include "keyboard/status_hid_protocol.h"
#include "keyboard/transport_routing.h"

using ai_keyboard::Action;
using ai_keyboard::ActionKind;
using ai_keyboard::FirmwareEventKind;
using ai_keyboard::InputPhase;

namespace {

constexpr const char* kHostAction =
    "host_action:123e4567-e89b-12d3-a456-426614174000";

std::string config_with_key1_action(const std::string& action) {
  return std::string(
             R"({"schema":"ai_keyboard.v1","profiles":[{"id":"default","keys":{"KEY1":{"press":")") +
         action +
         R"("},"KEY2":{"press":"disabled"},"KEY3":{"press":"disabled"},"KEY4":{"press":"disabled"},"KEY5":{"press":"disabled"},"KEY6":{"press":"disabled"},"KEY7":{"press":"disabled"},"KEY8":{"press":"disabled"}},"encoder":{"left":"disabled","right":"disabled","press":"disabled"}}]})";
}

std::string config_with_host_action_key(std::size_t key_index) {
  std::string json =
      R"({"schema":"ai_keyboard.v1","profiles":[{"id":"default","keys":{)";
  for (std::size_t index = 1; index <= 8; ++index) {
    if (index > 1) {
      json += ',';
    }
    json += "\"KEY" + std::to_string(index) + "\":{\"press\":\"";
    json += index == key_index ? kHostAction : "disabled";
    json += "\"}";
  }
  json +=
      R"(},"encoder":{"left":"disabled","right":"disabled","press":"disabled"}}]})";
  return json;
}

void parses_full_host_action_and_rejects_noncanonical_values() {
  const auto valid = ai_keyboard::parse_config_payload(
      config_with_key1_action(kHostAction));
  assert(valid.status == ai_keyboard::ConfigParseStatus::Ok);
  const auto& action = valid.config.keymap.action_for(ai_keyboard::InputId::Key1);
  assert(action.kind == ActionKind::HostAction);
  assert(action.hotkey == kHostAction);

  for (const auto* invalid : {
           "host_action:123E4567-e89b-12d3-a456-426614174000",
           "host_action:123e4567e89b-12d3-a456-426614174000",
           "host_action:123e4567-e89b-12d3-a456-42661417400",
           "host_action:123e4567-e89b-12d3-a456-42661417400g"}) {
    const auto result = ai_keyboard::parse_config_payload(
        config_with_key1_action(invalid));
    assert(result.status != ai_keyboard::ConfigParseStatus::Ok);
  }
}

void all_eight_keys_share_the_host_action_config_path() {
  for (std::size_t index = 1; index <= 8; ++index) {
    const auto result = ai_keyboard::parse_config_payload(
        config_with_host_action_key(index));
    assert(result.status == ai_keyboard::ConfigParseStatus::Ok);
    const auto input = static_cast<ai_keyboard::InputId>(index - 1);
    const auto& action = result.config.keymap.action_for(input);
    assert(action.kind == ActionKind::HostAction);
    assert(action.hotkey == kHostAction);
    const auto down = ai_keyboard::event_for_action(
        action, InputPhase::Pressed, "", "");
    const auto up = ai_keyboard::event_for_action(
        action, InputPhase::Released, "", "");
    assert(down.kind == FirmwareEventKind::HostAction);
    assert(down.value == kHostAction);
    assert(up.kind == FirmwareEventKind::None);
  }
}

void accepts_only_canonical_lowercase_uuid() {
  assert(ai_keyboard::is_canonical_host_action(kHostAction));
  assert(!ai_keyboard::is_canonical_host_action(
      "host_action:123E4567-e89b-12d3-a456-426614174000"));
  assert(!ai_keyboard::is_canonical_host_action(
      "host_action:123e4567e89b-12d3-a456-426614174000"));
  assert(!ai_keyboard::is_canonical_host_action(
      "host_action:123e4567-e89b-12d3-a456-42661417400"));
  assert(!ai_keyboard::is_canonical_host_action(
      "host_action:123e4567-e89b-12d3-a456-42661417400g"));
  assert(ai_keyboard::is_canonical_host_action(
      "host_action:00000000-0000-0000-0000-000000000000"));
  assert(ai_keyboard::is_canonical_host_action(
      "host_action:ffffffff-ffff-ffff-ffff-ffffffffffff"));
}

void host_action_events_press_once_and_release_none() {
  const Action action{ActionKind::HostAction, kHostAction};
  const auto down = ai_keyboard::event_for_action(
      action, InputPhase::Pressed, "", "");
  const auto up = ai_keyboard::event_for_action(
      action, InputPhase::Released, "", "");
  assert(down.kind == FirmwareEventKind::HostAction);
  assert(down.value == kHostAction);
  assert(up.kind == FirmwareEventKind::None);
  assert(up.value.empty());
}

void legacy_app_commands_keep_their_distinct_event_kind() {
  const Action action{ActionKind::OpenHistory, ""};
  const auto event = ai_keyboard::event_for_action(
      action, InputPhase::Pressed, "", "");
  assert(event.kind == FirmwareEventKind::AppCommand);
  assert(event.value == "history");
}

void encodes_fixed_host_action_wire_without_prefix() {
  std::array<std::uint8_t, ai_keyboard::kHostActionPayloadLen> payload{};
  assert(ai_keyboard::encode_host_action_app_command(kHostAction, &payload));
  assert(ai_keyboard::kHostActionCommandKind !=
         ai_keyboard::kStatusResponseCommandKind);
  assert(payload[0] == ai_keyboard::kHostActionCommandKind);
  assert(payload[1] == 0);
  assert(payload[2] == 1);
  assert(payload[3] == 36);
  const std::string uuid(reinterpret_cast<const char*>(payload.data() + 4), 36);
  assert(uuid == "123e4567-e89b-12d3-a456-426614174000");
  for (std::size_t index = 40; index < payload.size(); ++index) {
    assert(payload[index] == 0);
  }
}

void host_action_app_commands_keep_the_existing_usb_first_route() {
  const auto route = ai_keyboard::route_for_firmware_event(
      ai_keyboard::FirmwareEvent{FirmwareEventKind::HostAction, kHostAction},
      true);
  assert(route == ai_keyboard::FirmwareTransportRoute::UsbFirst);
}

void rejects_invalid_wire_input_without_partial_payload() {
  std::array<std::uint8_t, ai_keyboard::kHostActionPayloadLen> payload{};
  payload.fill(0xA5);
  assert(!ai_keyboard::encode_host_action_app_command(
      "host_action:123E4567-e89b-12d3-a456-426614174000", &payload));
  for (const auto byte : payload) {
    assert(byte == 0xA5);
  }
}

}  // namespace

int main() {
  parses_full_host_action_and_rejects_noncanonical_values();
  all_eight_keys_share_the_host_action_config_path();
  accepts_only_canonical_lowercase_uuid();
  host_action_events_press_once_and_release_none();
  legacy_app_commands_keep_their_distinct_event_kind();
  encodes_fixed_host_action_wire_without_prefix();
  host_action_app_commands_keep_the_existing_usb_first_route();
  rejects_invalid_wire_input_without_partial_payload();
  return 0;
}
