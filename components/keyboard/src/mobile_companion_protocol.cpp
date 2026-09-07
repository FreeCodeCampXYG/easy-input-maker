#include "keyboard/mobile_companion_protocol.h"

namespace ai_keyboard {
namespace {
std::uint16_t get16(const std::uint8_t* p) { return static_cast<std::uint16_t>(p[0]) | static_cast<std::uint16_t>(p[1] << 8); }
std::uint32_t get32(const std::uint8_t* p) { return p[0] | (static_cast<std::uint32_t>(p[1]) << 8) | (static_cast<std::uint32_t>(p[2]) << 16) | (static_cast<std::uint32_t>(p[3]) << 24); }
void put16(std::uint16_t v, std::uint8_t* p) { p[0] = static_cast<std::uint8_t>(v); p[1] = static_cast<std::uint8_t>(v >> 8); }
void put32(std::uint32_t v, std::uint8_t* p) { p[0] = static_cast<std::uint8_t>(v); p[1] = static_cast<std::uint8_t>(v >> 8); p[2] = static_cast<std::uint8_t>(v >> 16); p[3] = static_cast<std::uint8_t>(v >> 24); }
void finish(std::array<std::uint8_t, kMobileCompanionFrameLen>* out) { put16(mobile_companion_crc16(out->data(), 14), out->data() + 14); }
}

std::uint16_t mobile_companion_crc16(const std::uint8_t* data, std::size_t len) {
  std::uint16_t crc = 0xFFFF;
  for (std::size_t i = 0; i < len; ++i) { crc ^= data[i]; for (int b = 0; b < 8; ++b) crc = (crc & 1U) ? static_cast<std::uint16_t>((crc >> 1) ^ 0xA001) : static_cast<std::uint16_t>(crc >> 1); }
  return crc;
}

bool encode_mobile_companion_request(const MobileCompanionRequest& request,
                                     std::array<std::uint8_t, kMobileCompanionFrameLen>* out) {
  if (out == nullptr || request.request_id == 0 || request.capability > MobileCompanionCapability::Exclusive) return false;
  out->fill(0); (*out)[0] = kMobileCompanionMagic; (*out)[1] = kMobileCompanionProtocolVersion;
  (*out)[2] = static_cast<std::uint8_t>(MobileCompanionFrameType::Command);
  (*out)[3] = static_cast<std::uint8_t>(request.command); put16(request.request_id, out->data() + 4);
  (*out)[6] = 2; (*out)[7] = request.flags; put32(request.generation, out->data() + 8);
  (*out)[12] = static_cast<std::uint8_t>(request.capability);
  (*out)[13] = static_cast<std::uint8_t>(request.lease_ms / 1000U); finish(out); return true;
}

bool decode_mobile_companion_request(const std::uint8_t* data, std::size_t len, MobileCompanionRequest* out) {
  if (data == nullptr || out == nullptr || len != kMobileCompanionFrameLen || data[0] != kMobileCompanionMagic || data[1] != kMobileCompanionProtocolVersion || data[2] != static_cast<std::uint8_t>(MobileCompanionFrameType::Command) || data[3] < 1 || data[3] > 6 || data[6] > 2 || get16(data + 14) != mobile_companion_crc16(data, 14)) return false;
  out->frame_type = static_cast<MobileCompanionFrameType>(data[2]); out->command = static_cast<MobileCompanionCommand>(data[3]); out->request_id = get16(data + 4); out->flags = data[7]; out->generation = get32(data + 8); out->capability = static_cast<MobileCompanionCapability>(data[12]); out->lease_ms = static_cast<std::uint16_t>(data[13]) * 1000U;
  return out->capability <= MobileCompanionCapability::Exclusive &&
         (out->command == MobileCompanionCommand::QueryCapability ||
          out->command == MobileCompanionCommand::ReadConfig ||
          out->command == MobileCompanionCommand::WriteConfig ||
          out->capability == MobileCompanionCapability::Mirror);
}

bool encode_mobile_companion_ack(const MobileCompanionAck& ack, std::array<std::uint8_t, kMobileCompanionFrameLen>* out) {
  if (out == nullptr) return false; out->fill(0); (*out)[0] = kMobileCompanionMagic; (*out)[1] = kMobileCompanionProtocolVersion; (*out)[2] = static_cast<std::uint8_t>(ack.frame_type); (*out)[3] = static_cast<std::uint8_t>(ack.command); put16(ack.request_id, out->data() + 4); (*out)[6] = 2; (*out)[7] = ack.flags; put32(ack.generation, out->data() + 8); (*out)[12] = static_cast<std::uint8_t>(ack.result); (*out)[13] = static_cast<std::uint8_t>(ack.capability); finish(out); return true;
}

bool encode_mobile_companion_capability(MobileCompanionCapability capability, std::array<std::uint8_t, kMobileCompanionFrameLen>* out) { MobileCompanionAck ack; ack.frame_type = MobileCompanionFrameType::Capability; ack.result = MobileCompanionResult::Accepted; ack.capability = capability; return encode_mobile_companion_ack(ack, out); }

bool encode_mobile_companion_input_event(const MobileCompanionInputEvent& event, std::array<std::uint8_t, kMobileCompanionFrameLen>* out) {
  if (out == nullptr || event.input >= InputId::Count || event.event_id == 0) return false; out->fill(0); (*out)[0] = kMobileCompanionMagic; (*out)[1] = kMobileCompanionProtocolVersion; (*out)[2] = static_cast<std::uint8_t>(MobileCompanionFrameType::Event); put16(event.event_id, out->data() + 4); (*out)[6] = 2; (*out)[7] = event.flags | (event.phase == InputPhase::Pressed ? 1U : 2U); put32(event.generation, out->data() + 8); (*out)[12] = static_cast<std::uint8_t>(event.input); (*out)[13] = static_cast<std::uint8_t>(event.sequence); finish(out); return true;
}
}  // namespace ai_keyboard
