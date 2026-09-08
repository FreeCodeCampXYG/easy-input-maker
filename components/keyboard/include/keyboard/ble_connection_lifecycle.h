#pragma once

#include <cstdint>

namespace ai_keyboard {

constexpr std::uint16_t kBleInvalidConnectionHandle = 0xFFFF;

struct BleConnectionRoutingSnapshot {
  std::uint16_t active_conn_handle = kBleInvalidConnectionHandle;
  std::uint16_t control_conn_handle = kBleInvalidConnectionHandle;

  bool separate_control_connected() const {
    return control_conn_handle != kBleInvalidConnectionHandle &&
           control_conn_handle != active_conn_handle;
  }

  // 句柄是可复用的数值；广播策略只有在 NimBLE 仍能找到该连接时才可
  // 把它视为控制端，失败时必须清掉缓存，避免 stale handle 卡住广播。
  bool should_clear_stale_control(bool control_is_live) const {
    return separate_control_connected() && !control_is_live;
  }

  bool should_clear_disconnected_control(
      std::uint16_t disconnected_conn_handle) const {
    return separate_control_connected() &&
           disconnected_conn_handle == control_conn_handle;
  }
};

}  // namespace ai_keyboard
