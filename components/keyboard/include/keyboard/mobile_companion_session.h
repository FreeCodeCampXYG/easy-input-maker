#pragma once

#include "keyboard/mobile_companion_protocol.h"

namespace ai_keyboard {

enum class MobileCompanionSessionState : std::uint8_t {
  Disconnected, ConnectedUnverified, Encrypted, Bonded, ServiceReady,
  MirrorRequested, MirrorActive, Renewing, Releasing, Recovering, Expired,
};

class MobileCompanionSession {
 public:
  MobileCompanionAck handle(const MobileCompanionConnection& connection,
                            const MobileCompanionRequest& request,
                            bool encrypted, bool bonded, bool service_ready,
                            std::uint32_t now_ms);
  bool disconnect(const MobileCompanionConnection& connection);
  bool expire(std::uint32_t now_ms);
  bool accepts(const MobileCompanionConnection& connection, std::uint32_t now_ms) const;
  bool mirror_active(const MobileCompanionConnection& connection, std::uint32_t now_ms) const;
  MobileCompanionSessionState state() const { return state_; }
  std::uint32_t generation() const { return session_generation_; }

 private:
  bool deadline_reached(std::uint32_t now_ms) const;
  void clear(MobileCompanionSessionState state);
  MobileCompanionSessionState state_ = MobileCompanionSessionState::Disconnected;
  MobileCompanionConnection connection_{};
  std::uint32_t session_generation_ = 0;
  std::uint32_t deadline_ms_ = 0;
  std::uint16_t last_request_id_ = 0;
  MobileCompanionAck last_ack_{};
};

}  // namespace ai_keyboard
