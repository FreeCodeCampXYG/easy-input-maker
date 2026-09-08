#include <array>
#include <algorithm>
#include <cassert>
#include <string>

#include "keyboard/mobile_companion_config.h"

int main() {
  using namespace ai_keyboard;
  const std::string json = "{\"schema\":\"ai_keyboard.v1\",\"profiles\":[]}";
  MobileCompanionConfigAssembler assembler;
  const auto crc = mobile_companion_crc16(
      reinterpret_cast<const std::uint8_t*>(json.data()), json.size());
  const std::uint16_t total_chunks = static_cast<std::uint16_t>((json.size() + 1) / 2);
  for (std::uint16_t index = 0; index < total_chunks; ++index) {
    MobileCompanionConfigFragment fragment;
    fragment.command = MobileCompanionCommand::WriteConfig;
    fragment.request_id = 9;
    fragment.chunk_index = index;
    fragment.total_chunks = total_chunks;
    fragment.total_len = static_cast<std::uint16_t>(json.size());
    fragment.payload_crc = crc;
    fragment.data_len = static_cast<std::uint8_t>(
        std::min<std::size_t>(2, json.size() - static_cast<std::size_t>(index) * 2));
    fragment.data[0] = static_cast<std::uint8_t>(json[index * 2]);
    if (fragment.data_len > 1) fragment.data[1] = static_cast<std::uint8_t>(json[index * 2 + 1]);
    const auto result = assembler.receive(fragment);
    if (index + 1 < total_chunks) {
      assert(result.result == MobileCompanionConfigResult::Pending);
    } else {
      assert(result.result == MobileCompanionConfigResult::Complete);
      assert(result.json == json);
    }
  }
  MobileCompanionConfigFragment timeout{};
  timeout.request_id = 10;
  timeout.chunk_index = 0;
  timeout.total_chunks = 2;
  timeout.total_len = 3;
  timeout.payload_crc = 1;
  timeout.data_len = 2;
  assert(assembler.receive(timeout, 100).result == MobileCompanionConfigResult::Pending);
  assert(assembler.receive(timeout, 10201).result == MobileCompanionConfigResult::Invalid);
  return 0;
}
