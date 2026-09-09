#include <cassert>
#include <cstdint>
#include <string>

#include "keyboard/ble_status_wire.h"

namespace {

void appends_required_ble_fields_within_reserved_budget() {
  const std::string base =
      "{\"schema\":\"ai_keyboard.config_status.v1\",\"phase\":\"battery\"}";
  const auto wire = ai_keyboard::append_ble_status_wire_json(
      base,
      {
          true,
          true,
          UINT16_MAX,
          UINT16_MAX,
          UINT16_MAX,
      });

  assert(wire.size() <= ai_keyboard::kConfigStatusGattSafeLen);
  assert(wire.find(R"("ble":{"connected":1,"valid":1)") != std::string::npos);
  assert(wire.find(R"("idle")") == std::string::npos);
  assert(wire.find(R"("profile")") == std::string::npos);
  assert(wire.find(R"("valid":1)") != std::string::npos);
  assert(wire.find(R"("itvl":65535)") != std::string::npos);
  assert(wire.find(R"("latency":65535)") != std::string::npos);
  assert(wire.find(R"("timeout":65535)") != std::string::npos);
  assert(wire.size() - base.size() <=
         ai_keyboard::kConfigStatusBatteryBleReserveLen);
}

void maximum_reserved_base_still_accepts_worst_case_fragment() {
  const std::string minimum = "{\"x\":\"\"}";
  const auto base_len = ai_keyboard::kConfigStatusGattSafeLen -
                        ai_keyboard::kConfigStatusBatteryBleReserveLen;
  std::string base = "{\"x\":\"";
  base.append(base_len - minimum.size(), 'x');
  base += "\"}";
  assert(base.size() == base_len);

  const auto wire = ai_keyboard::append_ble_status_wire_json(
      base,
      {
          true,
          true,
          UINT16_MAX,
          UINT16_MAX,
          UINT16_MAX,
      });
  assert(wire.size() > base.size());
  assert(wire.size() <= ai_keyboard::kConfigStatusGattSafeLen);
  assert(wire.find(R"("ble":{)") != std::string::npos);
}

void invalid_or_unbudgeted_payload_is_left_unchanged() {
  const auto invalid =
      ai_keyboard::append_ble_status_wire_json("ready", {});
  assert(invalid == "ready");

  std::string oversized(ai_keyboard::kConfigStatusGattSafeLen, 'x');
  oversized.front() = '{';
  oversized.back() = '}';
  assert(ai_keyboard::append_ble_status_wire_json(oversized, {}) == oversized);
}

void diagnostic_status_exposes_latest_ble_lifecycle_snapshot() {
  ai_keyboard::ConfigStatusSnapshot snapshot;
  snapshot.firmware = "v";
  snapshot.phase = "diag";
  snapshot.status = "ok";
  snapshot.saved = true;
  snapshot.diagnostics.board = "v2";
  snapshot.diagnostics.ble_last_event =
      static_cast<std::uint8_t>(ai_keyboard::BleLifecycleEvent::EncChange);
  snapshot.diagnostics.ble_last_status = -22;
  snapshot.diagnostics.ble_last_handle = 7;
  snapshot.diagnostics.ble_event_sequence = 19;
  const auto wire = ai_keyboard::build_config_status_json(snapshot);
  assert(wire.size() <= ai_keyboard::kConfigStatusGattSafeLen);
  assert(wire.find(R"("ble_evt":3)") != std::string::npos);
  assert(wire.find(R"("ble_status":-22)") != std::string::npos);
  assert(wire.find(R"("ble_handle":7)") != std::string::npos);
  assert(wire.find(R"("ble_seq":19)") != std::string::npos);
}

}  // namespace

int main() {
  appends_required_ble_fields_within_reserved_budget();
  maximum_reserved_base_still_accepts_worst_case_fragment();
  invalid_or_unbudgeted_payload_is_left_unchanged();
  diagnostic_status_exposes_latest_ble_lifecycle_snapshot();
  return 0;
}
