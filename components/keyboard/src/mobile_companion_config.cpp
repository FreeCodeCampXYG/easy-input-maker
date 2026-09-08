#include "keyboard/mobile_companion_config.h"

#include <algorithm>
#include <utility>

namespace ai_keyboard {

MobileCompanionConfigReceiveResult MobileCompanionConfigAssembler::receive(
    const MobileCompanionConfigFragment& fragment,
    std::uint32_t now_ms) {
  if (active_ && now_ms != 0 && now_ms - last_activity_ms_ > 10000U) {
    reset();
    return {MobileCompanionConfigResult::Invalid, {}};
  }
  if (fragment.request_id == 0 || fragment.total_chunks == 0 ||
      fragment.chunk_index >= fragment.total_chunks || fragment.total_len == 0 ||
      fragment.total_len > kMobileCompanionConfigMaxBytes ||
      fragment.data_len > kMobileCompanionConfigFragmentDataLen) {
    return {MobileCompanionConfigResult::Invalid, {}};
  }
  if (!active_ && fragment.request_id == last_completed_request_id_) {
    return {MobileCompanionConfigResult::Duplicate, last_completed_json_};
  }
  if (!active_) {
    if (fragment.chunk_index != 0) {
      return {MobileCompanionConfigResult::OutOfOrder, {}};
    }
    reset();
    request_id_ = fragment.request_id;
    total_chunks_ = fragment.total_chunks;
    total_len_ = fragment.total_len;
    expected_crc_ = fragment.payload_crc;
    active_ = true;
  }
  if (now_ms != 0) last_activity_ms_ = now_ms;
  if (fragment.request_id != request_id_ || fragment.total_chunks != total_chunks_ ||
      fragment.total_len != total_len_ || fragment.payload_crc != expected_crc_) {
    return {MobileCompanionConfigResult::Invalid, {}};
  }
  if (fragment.chunk_index < next_chunk_) {
    return {MobileCompanionConfigResult::Duplicate, {}};
  }
  if (fragment.chunk_index != next_chunk_ || received_len_ + fragment.data_len > total_len_) {
    return {MobileCompanionConfigResult::OutOfOrder, {}};
  }
  std::copy_n(fragment.data.begin(), fragment.data_len, buffer_.begin() + received_len_);
  received_len_ = static_cast<std::uint16_t>(received_len_ + fragment.data_len);
  ++next_chunk_;
  if (next_chunk_ < total_chunks_) {
    return {MobileCompanionConfigResult::Pending, {}};
  }
  if (received_len_ != total_len_ || mobile_companion_crc16(buffer_.data(), received_len_) != expected_crc_) {
    reset();
    return {MobileCompanionConfigResult::CrcMismatch, {}};
  }
  std::string json(reinterpret_cast<const char*>(buffer_.data()), received_len_);
  last_completed_request_id_ = request_id_;
  last_completed_json_ = json;
  reset();
  return {MobileCompanionConfigResult::Complete, std::move(json)};
}

void MobileCompanionConfigAssembler::reset() {
  buffer_.fill(0);
  request_id_ = total_chunks_ = total_len_ = expected_crc_ = next_chunk_ = received_len_ = 0;
  last_activity_ms_ = 0;
  active_ = false;
}

}  // namespace ai_keyboard
