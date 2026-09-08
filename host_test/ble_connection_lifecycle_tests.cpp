#include <cassert>

#include "keyboard/ble_connection_lifecycle.h"

namespace {

using ai_keyboard::BleConnectionRoutingSnapshot;
using ai_keyboard::kBleInvalidConnectionHandle;

void test_live_auxiliary_connection_blocks_only_while_live() {
  const BleConnectionRoutingSnapshot snapshot{7, 9};
  assert(snapshot.separate_control_connected());
  assert(!snapshot.should_clear_stale_control(true));
  assert(!snapshot.should_clear_disconnected_control(7));
  assert(snapshot.should_clear_disconnected_control(9));
}

void test_stale_auxiliary_handle_is_reclaimed() {
  const BleConnectionRoutingSnapshot snapshot{7, 9};
  assert(snapshot.should_clear_stale_control(false));
}

void test_shared_hid_handle_is_never_treated_as_auxiliary() {
  const BleConnectionRoutingSnapshot snapshot{7, 7};
  assert(!snapshot.separate_control_connected());
  assert(!snapshot.should_clear_stale_control(false));
  assert(!snapshot.should_clear_disconnected_control(7));
}

void test_empty_snapshot_is_stable() {
  const BleConnectionRoutingSnapshot snapshot{};
  assert(snapshot.active_conn_handle == kBleInvalidConnectionHandle);
  assert(!snapshot.separate_control_connected());
  assert(!snapshot.should_clear_stale_control(false));
}

}  // namespace

int main() {
  test_live_auxiliary_connection_blocks_only_while_live();
  test_stale_auxiliary_handle_is_reclaimed();
  test_shared_hid_handle_is_never_treated_as_auxiliary();
  test_empty_snapshot_is_stable();
  return 0;
}
