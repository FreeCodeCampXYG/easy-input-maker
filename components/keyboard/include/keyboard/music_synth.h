#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace ai_keyboard {

constexpr std::size_t kMusicKeyCount = 8;
constexpr std::uint32_t kMusicSampleRate = 48000;
// 8 个实体键可同时按下；voice 池必须覆盖全部按键，否则释放尾音会占满
// voice 并让后续按键无法起音。混音仍按固定最大 voice 数留出削波余量。
constexpr std::size_t kMusicMaxVoices = kMusicKeyCount;

enum class MusicScale : std::uint8_t { Major, NaturalMinor, PentatonicMajor, Custom };
enum class MusicTimbre : std::uint8_t { SoftPiano, Organ, Bell, Pluck };
enum class DrumVoice : std::uint8_t { Kick, Snare, HiHat, Click };

struct MusicConfig {
  bool enabled = false;
  std::uint32_t reference_a4_milli_hz = 440000;
  std::int8_t root_midi = 60;
  std::int8_t transpose_semitones = 0;
  std::int16_t global_cents = 0;
  MusicScale scale = MusicScale::Major;
  std::array<std::int8_t, kMusicKeyCount> custom_offsets{{0, 2, 4, 5, 7, 9, 11, 12}};
  std::array<std::int16_t, kMusicKeyCount> key_cents{};
  std::array<MusicTimbre, kMusicKeyCount> timbres{};
  std::uint16_t attack_ms = 8;
  std::uint16_t release_ms = 45;
  std::uint16_t bpm = 120;
  std::uint8_t beats_per_bar = 4;
  std::uint8_t beat_unit = 4;
  bool metronome_enabled = false;
};

// MusicSynth 只保存离线可测的合成状态；平台层负责 I2S、GPIO8 和仲裁。
class MusicSynth {
 public:
  MusicSynth();
  bool apply_config(const MusicConfig& config);
  const MusicConfig& config() const;
  bool note_on(std::size_t key_index,
               std::uint8_t velocity_percent = 100,
               std::int8_t octave_offset = 0);
  bool note_off(std::size_t key_index, std::int8_t octave_offset = 0);
  bool trigger_drum(DrumVoice drum, std::uint8_t velocity_percent = 100);
  void render(std::int16_t* samples, std::size_t sample_count);
  bool active() const;
  bool metronome_running() const;
  // 返回本次 render 中是否跨过节拍；第一拍由 accented 标记。
  bool take_metronome_tick(bool* accented);

 private:
  enum class Envelope : std::uint8_t { Attack, Sustain, Release };
  struct Voice {
    bool active = false;
    std::size_t key_index = 0;
    std::int8_t octave_offset = 0;
    std::uint32_t phase = 0;
    std::uint32_t increment = 0;
    std::uint32_t age = 0;
    std::uint32_t level_q15 = 0;
    std::uint32_t velocity_q15 = 32767;
    Envelope envelope = Envelope::Attack;
    MusicTimbre timbre = MusicTimbre::SoftPiano;
    bool percussion = false;
    DrumVoice drum = DrumVoice::Click;
  };

  std::int32_t waveform(const Voice& voice) const;
  void advance_voice(Voice* voice);

  MusicConfig config_{};
  std::uint32_t attack_step_ = 1;
  std::uint32_t release_step_ = 1;
  std::array<Voice, kMusicMaxVoices> voices_{};
  std::uint64_t frame_clock_ = 0;
  std::uint64_t next_beat_frame_ = 0;
  bool metronome_tick_pending_ = false;
  bool metronome_tick_accented_ = false;
  std::uint32_t metronome_click_frames_remaining_ = 0;
  bool metronome_click_accented_ = false;
  std::uint32_t beat_index_ = 0;
};

struct TunerEstimate {
  bool valid = false;
  double frequency_hz = 0.0;
  double cents_from_a4 = 0.0;
};

bool validate_music_config(const MusicConfig& config);
TunerEstimate estimate_tuner_frequency(const std::int16_t* samples,
                                       std::size_t sample_count,
                                       std::uint32_t sample_rate = kMusicSampleRate);

std::int8_t music_key_semitone(const MusicConfig& config, std::size_t key_index);
double music_key_frequency_hz(const MusicConfig& config, std::size_t key_index);
std::uint32_t music_key_phase_increment(const MusicConfig& config, std::size_t key_index);

}  // namespace ai_keyboard
