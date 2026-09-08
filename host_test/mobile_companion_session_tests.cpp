#include <cassert>

#include "keyboard/mobile_companion_session.h"

int main() {
  using namespace ai_keyboard;
  MobileCompanionSession session;
  MobileCompanionRequest request;
  request.command = MobileCompanionCommand::RequestMirror;
  request.request_id = 1;
  request.capability = MobileCompanionCapability::Mirror;
  request.lease_ms = 1000;
  const MobileCompanionConnection first{1, 10};
  const MobileCompanionConnection reused{1, 11};
  assert(session.state() == MobileCompanionSessionState::Disconnected);
  assert(session.handle(first, request, true, true, true, 0).result == MobileCompanionResult::Accepted);
  assert(session.state() == MobileCompanionSessionState::MirrorActive);
  assert(session.handle(reused, request, true, true, true, 1).result == MobileCompanionResult::Busy);
  MobileCompanionRequest replay = request;
  replay.command = MobileCompanionCommand::QueryEvents;
  replay.request_id = 2;
  replay.generation = 999;
  assert(session.handle(first, replay, true, true, true, 1).result == MobileCompanionResult::StaleGeneration);
  assert(session.expire(1000));
  assert(session.state() == MobileCompanionSessionState::Expired);
  assert(!session.mirror_active(first, 1001));
  return 0;
}
