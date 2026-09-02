#include "keyboard/drum_sequencer.h"

#include <algorithm>
#include <array>

namespace ai_keyboard {
namespace {

constexpr std::uint16_t kMinDrumBpm = 20;
constexpr std::uint16_t kMaxDrumBpm = 300;
constexpr std::array<DrumVoice, 4> kFourFour{{
    DrumVoice::Kick, DrumVoice::HiHat, DrumVoice::Snare, DrumVoice::HiHat}};
constexpr std::array<DrumVoice, 3> kThreeFour{{
    DrumVoice::Kick, DrumVoice::HiHat, DrumVoice::Snare}};
constexpr std::array<DrumVoice, 6> kSixEight{{
    DrumVoice::Kick, DrumVoice::HiHat, DrumVoice::HiHat,
    DrumVoice::Snare, DrumVoice::HiHat, DrumVoice::HiHat}};
constexpr std::array<DrumVoice, 2> kTwoFour{{
    DrumVoice::Kick, DrumVoice::Snare}};
constexpr std::array<DrumVoice, 4> kSequential{{
    DrumVoice::Kick, DrumVoice::Snare, DrumVoice::HiHat, DrumVoice::Click}};

}  // namespace

void DrumSequencer::reset_clock() {
  frames_until_next_ = 0;
  frame_remainder_ = 0;
  step_index_ = 0;
}

void DrumSequencer::cycle_beat_mode() {
  // 首次按 K5 即启动节拍；后续只切拍号，不再插入“关闭”伪拍号。
  const bool first_start = !session_active();
  beat_enabled_ = true;
  if (!first_start) {
    switch (pattern_) {
      case DrumBeatPattern::BeatFour:
        pattern_ = DrumBeatPattern::BeatThree;
        break;
      case DrumBeatPattern::BeatThree:
        pattern_ = DrumBeatPattern::BeatSixEight;
        break;
      case DrumBeatPattern::BeatSixEight:
        pattern_ = DrumBeatPattern::BeatTwo;
        break;
      case DrumBeatPattern::BeatTwo:
        pattern_ = DrumBeatPattern::BeatFour;
        break;
    }
  }
  paused_ = false;
  step_index_ = 0;
  if (first_start) reset_clock();
}

void DrumSequencer::toggle_sequential() {
  const bool was_active = session_active();
  sequential_enabled_ = !sequential_enabled_;
  paused_ = false;
  step_index_ = 0;
  if (!was_active && sequential_enabled_) reset_clock();
}

void DrumSequencer::toggle_pause() {
  if (session_active()) paused_ = !paused_;
}

void DrumSequencer::stop() {
  beat_enabled_ = false;
  sequential_enabled_ = false;
  paused_ = false;
  reset_clock();
}

void DrumSequencer::adjust_bpm(int delta_bpm) {
  const auto adjusted = static_cast<int>(bpm_) + delta_bpm;
  bpm_ = static_cast<std::uint16_t>(std::clamp(
      adjusted, static_cast<int>(kMinDrumBpm), static_cast<int>(kMaxDrumBpm)));
}

std::optional<DrumVoice> DrumSequencer::advance(std::size_t sample_count) {
  if (!session_active() || paused_) return std::nullopt;
  if (frames_until_next_ > sample_count) {
    frames_until_next_ -= static_cast<std::uint32_t>(sample_count);
    return std::nullopt;
  }

  const auto beat_divisor =
      !sequential_enabled_ && pattern_ == DrumBeatPattern::BeatSixEight ? 2U : 1U;
  const auto step_denominator = static_cast<std::uint32_t>(bpm_) * beat_divisor;
  const auto duration_num = static_cast<std::uint64_t>(kMusicSampleRate) * 60U +
                            frame_remainder_;
  frames_until_next_ = static_cast<std::uint32_t>(duration_num / step_denominator);
  frame_remainder_ = static_cast<std::uint32_t>(duration_num % step_denominator);
  if (frames_until_next_ == 0U) frames_until_next_ = 1U;

  DrumVoice voice = DrumVoice::Click;
  if (sequential_enabled_) {
    voice = kSequential[step_index_ % kSequential.size()];
    step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kSequential.size());
  } else {
    switch (pattern_) {
      case DrumBeatPattern::BeatFour:
        voice = kFourFour[step_index_ % kFourFour.size()];
        step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kFourFour.size());
        break;
      case DrumBeatPattern::BeatThree:
        voice = kThreeFour[step_index_ % kThreeFour.size()];
        step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kThreeFour.size());
        break;
      case DrumBeatPattern::BeatSixEight:
        voice = kSixEight[step_index_ % kSixEight.size()];
        step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kSixEight.size());
        break;
      case DrumBeatPattern::BeatTwo:
        voice = kTwoFour[step_index_ % kTwoFour.size()];
        step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kTwoFour.size());
        break;
    }
  }
  return voice;
}

DrumBeatPattern DrumSequencer::pattern() const { return pattern_; }
bool DrumSequencer::paused() const { return paused_; }
bool DrumSequencer::session_active() const {
  return beat_enabled_ || sequential_enabled_;
}
bool DrumSequencer::sequential_enabled() const { return sequential_enabled_; }
std::uint16_t DrumSequencer::bpm() const { return bpm_; }

}  // namespace ai_keyboard
