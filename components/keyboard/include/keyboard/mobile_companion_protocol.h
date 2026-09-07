#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "keyboard/keymap.h"

namespace ai_keyboard {

inline constexpr std::uint8_t kMobileCompanionMagic = 0xE1;
inline constexpr std::uint8_t kMobileCompanionProtocolVersion = 1;
inline constexpr std::size_t kMobileCompanionFrameLen = 16;
inline constexpr std::uint32_t kMobileCompanionDefaultLeaseMs = 15000;
inline constexpr std::uint32_t kMobileCompanionMaxLeaseMs = 30000;

// 0 magic, 1 version, 2 type, 3 command, 4..5 request_id, 6 payload_length,
// 7 flags, 8..11 generation, 12..13 payload, 14..15 CRC16 (little-endian).
enum class MobileCompanionFrameType : std::uint8_t { Command = 1, Ack = 2, Capability = 3, Event = 4, Config = 5 };
enum class MobileCompanionCommand : std::uint8_t {
  RequestMirror = 1, ReleaseMirror = 2, RenewMirror = 3, QueryCapability = 4,
  ReadConfig = 5, WriteConfig = 6,
};
enum class MobileCompanionCapability : std::uint8_t { Unsupported = 0, Mirror = 1, Exclusive = 2 };
enum class MobileCompanionResult : std::uint8_t {
  Accepted = 0, UnsupportedVersion = 1, InvalidFrame = 2, Unauthorized = 3,
  NotEncrypted = 4, NotBonded = 5, SessionExpired = 6, StaleGeneration = 7,
  DuplicateRequest = 8, Busy = 9, UnsupportedCommand = 10,
};

inline constexpr std::uint8_t kMobileFlagExplicitExclusive = 0x01;
inline constexpr std::uint8_t kMobileFlagKey1Voice = 0x10;
inline constexpr std::uint8_t kMobileFlagKey3Rewrite = 0x20;
inline constexpr std::uint8_t kMobileFlagKey8Shortcut = 0x40;

struct MobileCompanionConnection {
  static constexpr std::uint16_t kNoConnection = 0xFFFF;
  std::uint16_t conn_handle = kNoConnection;
  std::uint32_t generation = 0;
  bool valid() const { return conn_handle != kNoConnection && generation != 0; }
  bool operator==(const MobileCompanionConnection& other) const { return conn_handle == other.conn_handle && generation == other.generation; }
  bool operator!=(const MobileCompanionConnection& other) const { return !(*this == other); }
};

struct MobileCompanionRequest {
  MobileCompanionFrameType frame_type = MobileCompanionFrameType::Command;
  MobileCompanionCommand command = MobileCompanionCommand::QueryCapability;
  std::uint16_t request_id = 0;
  std::uint8_t flags = 0;
  std::uint32_t generation = 0;
  MobileCompanionCapability capability = MobileCompanionCapability::Unsupported;
  std::uint16_t lease_ms = 0;
};

struct MobileCompanionAck {
  MobileCompanionFrameType frame_type = MobileCompanionFrameType::Ack;
  MobileCompanionCommand command = MobileCompanionCommand::QueryCapability;
  std::uint16_t request_id = 0;
  std::uint8_t flags = 0;
  std::uint32_t generation = 0;
  MobileCompanionResult result = MobileCompanionResult::InvalidFrame;
  MobileCompanionCapability capability = MobileCompanionCapability::Unsupported;
  std::uint16_t lease_ms = 0;
};

struct MobileCompanionInputEvent {
  std::uint16_t event_id = 0;
  InputId input = InputId::Count;
  InputPhase phase = InputPhase::Pressed;
  std::uint8_t flags = 0;
  std::uint32_t generation = 0;
  std::uint32_t sequence = 0;
};

std::uint16_t mobile_companion_crc16(const std::uint8_t* data, std::size_t len);
bool encode_mobile_companion_request(const MobileCompanionRequest& request,
                                     std::array<std::uint8_t, kMobileCompanionFrameLen>* out);
bool decode_mobile_companion_request(const std::uint8_t* data, std::size_t len, MobileCompanionRequest* out);
bool encode_mobile_companion_ack(const MobileCompanionAck& ack, std::array<std::uint8_t, kMobileCompanionFrameLen>* out);
bool encode_mobile_companion_capability(MobileCompanionCapability capability, std::array<std::uint8_t, kMobileCompanionFrameLen>* out);
bool encode_mobile_companion_input_event(const MobileCompanionInputEvent& event, std::array<std::uint8_t, kMobileCompanionFrameLen>* out);

}  // namespace ai_keyboard
