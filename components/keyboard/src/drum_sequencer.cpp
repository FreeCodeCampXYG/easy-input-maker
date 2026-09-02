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
constexpr std::array<DrumVoice, 4> kSequential{{
    DrumVoice::Kick, DrumVoice::Snare, DrumVoice::HiHat, DrumVoice::Click}};

}  // namespace

void DrumSequencer::reset_clock() {
  frames_until_next_ = 0;
  frame_remainder_ = 0;
  step_index_ = 0;
}

void DrumSequencer::cycle_beat_mode() {
  switch (mode_) {
    case DrumLoopMode::Off:
    case DrumLoopMode::Sequential:
      mode_ = DrumLoopMode::BeatFour;
      break;
    case DrumLoopMode::BeatFour:
      mode_ = DrumLoopMode::BeatThree;
      break;
    case DrumLoopMode::BeatThree:
      mode_ = DrumLoopMode::Off;
      break;
  }
  paused_ = false;
  reset_clock();
}

void DrumSequencer::toggle_sequential() {
  mode_ = mode_ == DrumLoopMode::Sequential ? DrumLoopMode::Off
                                              : DrumLoopMode::Sequential;
  paused_ = false;
  reset_clock();
}

void DrumSequencer::toggle_pause() {
  if (session_active()) paused_ = !paused_;
}

void DrumSequencer::stop() {
  mode_ = DrumLoopMode::Off;
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

  const auto duration_num = static_cast<std::uint64_t>(kMusicSampleRate) * 60U +
                            frame_remainder_;
  frames_until_next_ = static_cast<std::uint32_t>(duration_num / bpm_);
  frame_remainder_ = static_cast<std::uint32_t>(duration_num % bpm_);
  if (frames_until_next_ == 0U) frames_until_next_ = 1U;

  DrumVoice voice = DrumVoice::Click;
  switch (mode_) {
    case DrumLoopMode::BeatFour:
      voice = kFourFour[step_index_ % kFourFour.size()];
      step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kFourFour.size());
      break;
    case DrumLoopMode::BeatThree:
      voice = kThreeFour[step_index_ % kThreeFour.size()];
      step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kThreeFour.size());
      break;
    case DrumLoopMode::Sequential:
      voice = kSequential[step_index_ % kSequential.size()];
      step_index_ = static_cast<std::uint8_t>((step_index_ + 1U) % kSequential.size());
      break;
    case DrumLoopMode::Off:
      return std::nullopt;
  }
  return voice;
}

DrumLoopMode DrumSequencer::mode() const { return mode_; }
bool DrumSequencer::paused() const { return paused_; }
bool DrumSequencer::session_active() const { return mode_ != DrumLoopMode::Off; }
std::uint16_t DrumSequencer::bpm() const { return bpm_; }

}  // namespace ai_keyboard
