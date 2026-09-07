#include "keyboard/mobile_companion_session.h"

#include <algorithm>

namespace ai_keyboard {

MobileCompanionAck MobileCompanionSession::handle(
    const MobileCompanionConnection& connection,
    const MobileCompanionRequest& request,
    bool encrypted,
    bool bonded,
    bool service_ready,
    std::uint32_t now_ms) {
  MobileCompanionAck ack;
  ack.command = request.command;
  ack.request_id = request.request_id;
  ack.generation = connection.generation;
  if (!connection.valid()) {
    ack.result = MobileCompanionResult::StaleGeneration;
    return ack;
  }
  if ((request.command == MobileCompanionCommand::RenewMirror ||
       request.command == MobileCompanionCommand::ReleaseMirror) &&
      request.generation != 0 && request.generation != session_generation_) {
    state_ = MobileCompanionSessionState::Recovering;
    ack.result = MobileCompanionResult::StaleGeneration;
    return ack;
  }
  if (!encrypted) {
    state_ = MobileCompanionSessionState::ConnectedUnverified;
    ack.result = MobileCompanionResult::NotEncrypted;
    return ack;
  }
  // 不先改写当前会话状态；第二条连接必须先被识别为 Busy，避免覆盖活跃代际。
  if (connection_.valid() && connection_ != connection &&
      state_ == MobileCompanionSessionState::MirrorActive &&
      !deadline_reached(now_ms)) {
    ack.result = MobileCompanionResult::Busy;
    return ack;
  }
  state_ = bonded ? (service_ready ? MobileCompanionSessionState::ServiceReady
                                   : MobileCompanionSessionState::Bonded)
                  : MobileCompanionSessionState::Encrypted;
  if (!bonded) {
    ack.result = MobileCompanionResult::NotBonded;
    return ack;
  }
  if (!service_ready) {
    ack.result = MobileCompanionResult::InvalidFrame;
    return ack;
  }
  if (request.request_id != 0 && request.request_id == last_request_id_ &&
      connection == connection_) {
    return last_ack_;
  }
  if ((request.command == MobileCompanionCommand::RenewMirror ||
       request.command == MobileCompanionCommand::ReleaseMirror) &&
      !accepts(connection, now_ms)) {
    ack.result = MobileCompanionResult::SessionExpired;
    return ack;
  }
  if (request.command == MobileCompanionCommand::QueryCapability) {
    ack.result = MobileCompanionResult::Accepted;
    ack.capability = MobileCompanionCapability::Mirror;
  } else if (request.command == MobileCompanionCommand::RequestMirror ||
             request.command == MobileCompanionCommand::RenewMirror) {
    if (request.capability != MobileCompanionCapability::Mirror) {
      ack.result = MobileCompanionResult::Unauthorized;
    } else {
      if (!accepts(connection, now_ms)) {
        ++session_generation_;
        if (session_generation_ == 0) {
          ++session_generation_;
        }
      }
      connection_ = connection;
      state_ = request.command == MobileCompanionCommand::RenewMirror
                   ? MobileCompanionSessionState::Renewing
                   : MobileCompanionSessionState::MirrorRequested;
      const auto lease = std::min<std::uint32_t>(
          request.lease_ms == 0 ? kMobileCompanionDefaultLeaseMs
                                : request.lease_ms,
          kMobileCompanionMaxLeaseMs);
      deadline_ms_ = now_ms + lease;
      state_ = MobileCompanionSessionState::MirrorActive;
      ack.result = MobileCompanionResult::Accepted;
      ack.capability = MobileCompanionCapability::Mirror;
      ack.lease_ms = static_cast<std::uint16_t>(lease);
      ack.generation = session_generation_;
    }
  } else if (request.command == MobileCompanionCommand::ReleaseMirror) {
    clear(MobileCompanionSessionState::ServiceReady);
    ack.result = MobileCompanionResult::Accepted;
  } else {
    ack.result = MobileCompanionResult::UnsupportedCommand;
  }
  last_request_id_ = request.request_id;
  last_ack_ = ack;
  return ack;
}

bool MobileCompanionSession::disconnect(
    const MobileCompanionConnection& connection) {
  if (connection_ != connection) {
    return false;
  }
  clear(MobileCompanionSessionState::Disconnected);
  return true;
}

bool MobileCompanionSession::expire(std::uint32_t now_ms) {
  if (state_ != MobileCompanionSessionState::MirrorActive ||
      !deadline_reached(now_ms)) {
    return false;
  }
  clear(MobileCompanionSessionState::Expired);
  return true;
}

bool MobileCompanionSession::accepts(
    const MobileCompanionConnection& connection,
    std::uint32_t now_ms) const {
  return state_ == MobileCompanionSessionState::MirrorActive &&
         connection_ == connection && !deadline_reached(now_ms);
}

bool MobileCompanionSession::mirror_active(
    const MobileCompanionConnection& connection,
    std::uint32_t now_ms) const {
  return accepts(connection, now_ms);
}

bool MobileCompanionSession::deadline_reached(std::uint32_t now_ms) const {
  return static_cast<std::int32_t>(now_ms - deadline_ms_) >= 0;
}

void MobileCompanionSession::clear(MobileCompanionSessionState state) {
  connection_ = {};
  deadline_ms_ = 0;
  state_ = state;
}

}  // namespace ai_keyboard
