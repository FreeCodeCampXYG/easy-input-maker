#include "keyboard/music_sequence_player.h"

namespace ai_keyboard {

bool MusicSequencePlayer::load(const MusicSequence& sequence) {
  if (sequence.command != MusicSequenceCommand::Play || sequence.event_count == 0 ||
      sequence.bpm < 20 || sequence.bpm > 300) {
    return false;
  }
  sequence_ = sequence;
  event_index_ = 0;
  event_frames_remaining_ = 0;
  frame_remainder_ = 0;
  playing_ = true;
  return true;
}

void MusicSequencePlayer::stop(MusicSynth* synth) {
  if (synth != nullptr && event_index_ < sequence_.event_count &&
      sequence_.events[event_index_].degree != kMusicSequenceRest) {
    synth->note_off(sequence_.events[event_index_].degree);
  }
  playing_ = false;
  event_frames_remaining_ = 0;
}

void MusicSequencePlayer::advance(std::size_t sample_count, MusicSynth* synth) {
  if (!playing_ || synth == nullptr) return;
  const auto frames_per_sixteenth_num =
      static_cast<std::uint64_t>(kMusicSampleRate) * 60U;
  const auto frames_per_sixteenth_den = static_cast<std::uint64_t>(sequence_.bpm) * 4U;
  std::size_t remaining = sample_count;
  while (remaining != 0U && playing_) {
    if (event_frames_remaining_ == 0U) {
      if (event_index_ >= sequence_.event_count) {
        stop(synth);
        break;
      }
      const auto& event = sequence_.events[event_index_];
      if (event.degree != kMusicSequenceRest) {
        synth->note_on(event.degree, event.velocity_percent);
      }
      const auto duration_num = frames_per_sixteenth_num * event.duration_sixteenths + frame_remainder_;
      event_frames_remaining_ = static_cast<std::uint32_t>(duration_num / frames_per_sixteenth_den);
      frame_remainder_ = duration_num % frames_per_sixteenth_den;
      if (event_frames_remaining_ == 0U) event_frames_remaining_ = 1U;
    }
    const auto consumed = remaining < event_frames_remaining_ ? remaining : event_frames_remaining_;
    event_frames_remaining_ -= static_cast<std::uint32_t>(consumed);
    remaining -= consumed;
    if (event_frames_remaining_ == 0U) {
      const auto& event = sequence_.events[event_index_];
      if (event.degree != kMusicSequenceRest) synth->note_off(event.degree);
      ++event_index_;
      if (event_index_ >= sequence_.event_count) {
        playing_ = false;
      }
    }
  }
}

bool MusicSequencePlayer::playing() const { return playing_; }

}  // namespace ai_keyboard
