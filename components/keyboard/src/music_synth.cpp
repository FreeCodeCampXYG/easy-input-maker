#include "keyboard/music_synth.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ai_keyboard {
namespace {

constexpr std::uint32_t kMetronomeClickFrames = kMusicSampleRate / 50U;

constexpr std::array<std::int8_t, kMusicKeyCount> kMajor{{0, 2, 4, 5, 7, 9, 11, 12}};
constexpr std::array<std::int8_t, kMusicKeyCount> kNaturalMinor{{0, 2, 3, 5, 7, 8, 10, 12}};
constexpr std::array<std::int8_t, kMusicKeyCount> kPentatonicMajor{{0, 2, 4, 7, 9, 12, 14, 16}};

std::uint32_t drum_increment(DrumVoice drum) {
  switch (drum) {
    case DrumVoice::Kick: return 120000000U;
    case DrumVoice::Snare: return 700000000U;
    case DrumVoice::HiHat: return 1400000000U;
    case DrumVoice::Click: return 1800000000U;
  }
  return 0U;
}

const std::array<std::int8_t, kMusicKeyCount>& scale_offsets(MusicScale scale) {
  switch (scale) {
    case MusicScale::Major: return kMajor;
    case MusicScale::NaturalMinor: return kNaturalMinor;
    case MusicScale::PentatonicMajor: return kPentatonicMajor;
    case MusicScale::Custom: return kMajor;
  }
  return kMajor;
}

}  // namespace

bool validate_music_config(const MusicConfig& config) {
  // 未知枚举不能落入大调兜底，否则错误配置会被当成合法音阶保存。
  if (static_cast<std::uint8_t>(config.scale) >
      static_cast<std::uint8_t>(MusicScale::Custom)) return false;
  if (config.reference_a4_milli_hz < 400000 ||
      config.reference_a4_milli_hz > 480000 || config.root_midi < 0 ||
      config.transpose_semitones < -36 ||
      config.transpose_semitones > 36 || config.global_cents < -1200 ||
      config.global_cents > 1200 || config.attack_ms > 2000 ||
      config.release_ms > 4000 || config.bpm < 20 || config.bpm > 300) {
    return false;
  }
  const bool meter_ok = (config.beats_per_bar == 2 && config.beat_unit == 4) ||
                        (config.beats_per_bar == 3 && config.beat_unit == 4) ||
                        (config.beats_per_bar == 4 && config.beat_unit == 4) ||
                        (config.beats_per_bar == 6 && config.beat_unit == 8);
  if (!meter_ok) return false;
  for (const auto cents : config.key_cents) {
    if (cents < -1200 || cents > 1200) return false;
  }
  for (const auto timbre : config.timbres) {
    if (static_cast<std::uint8_t>(timbre) > static_cast<std::uint8_t>(MusicTimbre::Pluck)) return false;
  }
  return true;
}

std::int8_t music_key_semitone(const MusicConfig& config, std::size_t key_index) {
  if (key_index >= kMusicKeyCount) return 0;
  return config.scale == MusicScale::Custom ? config.custom_offsets[key_index]
                                             : scale_offsets(config.scale)[key_index];
}

double music_key_frequency_hz(const MusicConfig& config, std::size_t key_index) {
  if (key_index >= kMusicKeyCount || config.reference_a4_milli_hz == 0) return 0.0;
  const auto midi = static_cast<std::int32_t>(config.root_midi) +
                    static_cast<std::int32_t>(config.transpose_semitones) +
                    static_cast<std::int32_t>(music_key_semitone(config, key_index));
  const auto cents = static_cast<std::int32_t>(config.global_cents) +
                     static_cast<std::int32_t>(config.key_cents[key_index]);
  const double a4 = static_cast<double>(config.reference_a4_milli_hz) / 1000.0;
  return a4 * std::exp2((static_cast<double>(midi - 69) +
                         static_cast<double>(cents) / 100.0) / 12.0);
}

std::uint32_t music_key_phase_increment(const MusicConfig& config, std::size_t key_index) {
  const double frequency = music_key_frequency_hz(config, key_index);
  if (frequency <= 0.0) return 0;
  const double phase = frequency * 4294967296.0 / static_cast<double>(kMusicSampleRate);
  return static_cast<std::uint32_t>(std::llround(std::min(phase, 4294967295.0)));
}

MusicSynth::MusicSynth() {
  apply_config(config_);
}

bool MusicSynth::apply_config(const MusicConfig& config) {
  if (!validate_music_config(config)) return false;
  config_ = config;
  // 配置只由音频 owner 应用；缓存原整数步长，逐样本 PCM 舍入保持不变。
  const auto attack_frames = std::max<std::uint32_t>(1, config.attack_ms * 48U);
  const auto release_frames = std::max<std::uint32_t>(1, config.release_ms * 48U);
  attack_step_ = 32767U / attack_frames + 1U;
  release_step_ = std::max<std::uint32_t>(1, 32767U / release_frames);
  return true;
}

const MusicConfig& MusicSynth::config() const { return config_; }

bool MusicSynth::note_on(std::size_t key_index,
                         std::uint8_t velocity_percent,
                         std::int8_t octave_offset) {
  if (!config_.enabled || key_index >= kMusicKeyCount ||
      octave_offset < -1 || octave_offset > 1) return false;
  Voice* selected = nullptr;
  for (auto& voice : voices_) {
    if (voice.active && !voice.percussion && voice.key_index == key_index &&
        voice.octave_offset == octave_offset) {
      selected = &voice;
      break;
    }
    if (!voice.active && selected == nullptr) selected = &voice;
  }
  if (selected == nullptr) {
    selected = &voices_[0];
    for (auto& voice : voices_) if (voice.age > selected->age) selected = &voice;
  }
  // 重触发沿用旧相位，避免波形在非零点硬切造成爆音。
  const auto old_phase = selected->phase;
  *selected = {};
  selected->active = true;
  selected->key_index = key_index;
  selected->octave_offset = octave_offset;
  selected->phase = old_phase;
  selected->increment = music_key_phase_increment(config_, key_index);
  if (octave_offset < 0) {
    selected->increment >>= static_cast<unsigned>(-octave_offset);
  } else if (octave_offset > 0) {
    selected->increment <<= static_cast<unsigned>(octave_offset);
  }
  selected->timbre = config_.timbres[key_index];
  selected->velocity_q15 = static_cast<std::uint32_t>(
      std::min<std::uint8_t>(velocity_percent, 100U)) * 32767U / 100U;
  selected->envelope = Envelope::Attack;
  return selected->increment != 0;
}

bool MusicSynth::note_off(std::size_t key_index, std::int8_t octave_offset) {
  bool changed = false;
  // 鼓声共用 voice 池但不属于琴键，不能被默认 key_index=0 的松键截断。
  for (auto& voice : voices_) {
    if (voice.active && !voice.percussion && voice.key_index == key_index &&
        voice.octave_offset == octave_offset) {
      voice.envelope = Envelope::Release;
      changed = true;
    }
  }
  return changed;
}

bool MusicSynth::trigger_drum(DrumVoice drum, std::uint8_t velocity_percent) {
  Voice* selected = nullptr;
  for (auto& voice : voices_) if (!voice.active) { selected = &voice; break; }
  if (selected == nullptr) selected = &voices_[0];
  *selected = {};
  selected->active = true;
  selected->percussion = true;
  selected->drum = drum;
  selected->increment = drum_increment(drum);
  selected->velocity_q15 = static_cast<std::uint32_t>(
      std::min<std::uint8_t>(velocity_percent, 100U)) * 32767U / 100U;
  selected->level_q15 = 32767U;
  selected->envelope = Envelope::Release;
  return true;
}

std::int32_t MusicSynth::waveform(const Voice& voice) const {
  if (voice.percussion) {
    std::uint32_t noise = voice.phase ^ (voice.phase >> 7U) ^ (voice.phase >> 13U);
    const auto sample = static_cast<std::int32_t>(noise & 0xFFFFU) - 32768;
    if (voice.drum == DrumVoice::Kick) {
      return static_cast<std::int32_t>((32767LL * (32767 - static_cast<std::int32_t>(voice.phase >> 17U))) / 32767LL);
    }
    if (voice.drum == DrumVoice::Click) return sample / 2;
    return sample;
  }
  const std::int32_t saw = static_cast<std::int32_t>(voice.phase >> 16U) - 32768;
  const std::int32_t triangle = 32767 - std::abs(saw * 2);
  switch (voice.timbre) {
    case MusicTimbre::Organ: return saw;
    case MusicTimbre::Bell: return (triangle * triangle) / 32767;
    case MusicTimbre::Pluck: return saw > 0 ? 12000 : -12000;
    case MusicTimbre::SoftPiano: return triangle;
  }
  return 0;
}

void MusicSynth::advance_voice(Voice* voice) {
  if (voice == nullptr || !voice->active) return;
  if (voice->envelope == Envelope::Attack) {
    voice->level_q15 = std::min<std::uint32_t>(32767, voice->level_q15 + attack_step_);
    if (voice->level_q15 >= 32767) voice->envelope = Envelope::Sustain;
  } else if (voice->envelope == Envelope::Release) {
    if (voice->level_q15 <= release_step_) {
      voice->active = false;
      return;
    }
    voice->level_q15 -= release_step_;
  }
  voice->phase += voice->increment;
  ++voice->age;
}

void MusicSynth::render(std::int16_t* samples, std::size_t sample_count) {
  if (samples == nullptr) return;
  // BPM 沿用四分音符口径；6/8 的每个 click 应只占半拍。
  const std::uint64_t frames_per_beat =
      std::max<std::uint64_t>(1, (static_cast<std::uint64_t>(kMusicSampleRate) * 60U * 4U) /
                                   (config_.bpm * config_.beat_unit));
  for (std::size_t index = 0; index < sample_count; ++index) {
    std::int64_t mixed = 0;
    for (auto& voice : voices_) {
      if (!voice.active) continue;
      mixed += (static_cast<std::int64_t>(waveform(voice)) * voice.level_q15 *
                voice.velocity_q15) / (32767LL * 32767LL);
      advance_voice(&voice);
    }
    // 固定按最大复音数留出余量，避免和弦相加后硬削波；固定分母也让
    // 加入/释放一个声部时响度不会突然跳变，保留失败时的可听数据。
    mixed /= static_cast<std::int64_t>(kMusicMaxVoices);
    if (config_.enabled && config_.metronome_enabled &&
        frame_clock_ >= next_beat_frame_) {
      metronome_tick_pending_ = true;
      metronome_tick_accented_ = beat_index_ == 0;
      metronome_click_frames_remaining_ = kMetronomeClickFrames;
      metronome_click_accented_ = metronome_tick_accented_;
      beat_index_ = (beat_index_ + 1) % config_.beats_per_bar;
      next_beat_frame_ = frame_clock_ + frames_per_beat;
    }
    if (metronome_click_frames_remaining_ != 0U) {
      const auto phase = (kMetronomeClickFrames - metronome_click_frames_remaining_) % 24U;
      const auto polarity = phase < 12U ? 1 : -1;
      const auto peak = metronome_click_accented_ ? 9000 : 5500;
      mixed += polarity *
               (static_cast<std::int64_t>(peak) * metronome_click_frames_remaining_) /
                   kMetronomeClickFrames;
      --metronome_click_frames_remaining_;
    }
    const auto limited = std::max<std::int64_t>(-32768, std::min<std::int64_t>(32767, mixed));
    samples[index] = static_cast<std::int16_t>(limited);
    ++frame_clock_;
  }
}

bool MusicSynth::active() const {
  for (const auto& voice : voices_) if (voice.active) return true;
  return false;
}

bool MusicSynth::metronome_running() const {
  return config_.enabled && config_.metronome_enabled;
}

bool MusicSynth::take_metronome_tick(bool* accented) {
  if (!metronome_tick_pending_) return false;
  if (accented != nullptr) *accented = metronome_tick_accented_;
  metronome_tick_pending_ = false;
  return true;
}

TunerEstimate estimate_tuner_frequency(const std::int16_t* samples,
                                       std::size_t sample_count,
                                       std::uint32_t sample_rate) {
  if (samples == nullptr || sample_rate == 0 || sample_count < 192) return {};
  const std::size_t min_lag = std::max<std::size_t>(1, sample_rate / 1200U);
  const std::size_t max_lag = std::min<std::size_t>(sample_count / 2U, sample_rate / 60U);
  std::int64_t best_score = std::numeric_limits<std::int64_t>::min();
  std::size_t best_lag = 0;
  for (std::size_t lag = min_lag; lag <= max_lag; ++lag) {
    std::int64_t score = 0;
    for (std::size_t index = 0; index + lag < sample_count; ++index) {
      score += static_cast<std::int32_t>(samples[index]) * samples[index + lag];
    }
    if (score > best_score) { best_score = score; best_lag = lag; }
  }
  if (best_lag == 0 || best_score <= 0) return {};
  const double frequency = static_cast<double>(sample_rate) / best_lag;
  return {true, frequency, 1200.0 * std::log2(frequency / 440.0)};
}

}  // namespace ai_keyboard
