#include <array>
#include <cassert>

#include "keyboard/music_sequence_player.h"
#include "keyboard/music_builtin.h"

namespace {

void protocol_round_trip_and_player_progress() {
  ai_keyboard::MusicSequence sequence;
  sequence.command = ai_keyboard::MusicSequenceCommand::Play;
  sequence.bpm = 120;
  sequence.event_count = 2;
  sequence.events[0] = {0, 4, 100};
  sequence.events[1] = {2, 4, 45};
  std::array<std::uint8_t, ai_keyboard::kMusicSequencePayloadLen> payload{};
  assert(ai_keyboard::encode_music_sequence(sequence, &payload));
  const auto decoded = ai_keyboard::decode_music_sequence(payload.data(), payload.size());
  assert(decoded.has_value() && decoded->event_count == 2 &&
         decoded->events[1].degree == 2 && decoded->events[1].velocity_percent >= 40 &&
         decoded->events[1].velocity_percent <= 50);

  ai_keyboard::MusicSynth synth;
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  assert(synth.apply_config(config));
  ai_keyboard::MusicSequencePlayer player;
  assert(player.load(*decoded));
  // 当前 wire 将时值编码为十六分拍数，4 代表一拍；120 BPM 下每个
  // 事件为 24000 帧，两个事件合计 48000 帧。
  player.advance(48000, &synth);
  assert(!player.playing());
}

void builtin_ode_to_joy_is_compact_and_playable() {
  const auto sequence = ai_keyboard::builtin_music_sequence(
      ai_keyboard::kBuiltinSongOdeToJoy);
  assert(sequence.has_value());
  assert(sequence->command == ai_keyboard::MusicSequenceCommand::Play);
  assert(sequence->event_count == 96);
  assert(sequence->event_count <= ai_keyboard::kMusicSequenceStorageEvents);
  assert(sequence->events[0].degree == 2);
  assert(sequence->events[12].duration_sixteenths == 6);
  assert(sequence->events[14].duration_sixteenths == 8);
  assert(sequence->events[15].degree == 2);
  assert(sequence->events[29].duration_sixteenths == 8);
  assert(sequence->events[30].degree == 1);
  // 展开段第 2、3 小节的“3 4”是八分音符对（十六分拍数为 2）。
  assert(sequence->events[35].degree == 2 &&
         sequence->events[35].duration_sixteenths == 2);
  assert(sequence->events[36].degree == 3 &&
         sequence->events[36].duration_sixteenths == 2);
  assert(sequence->events[40].degree == 2 &&
         sequence->events[40].duration_sixteenths == 2);
  assert(sequence->events[41].degree == 3 &&
         sequence->events[41].duration_sixteenths == 2);
  assert(sequence->events[46].degree == 4);
  assert(sequence->events[46].octave_offset == -1);
  assert(sequence->events[47].degree == 2);
  assert(sequence->events[48].degree == 2);
  assert(sequence->events[95].duration_sixteenths == 8);
  assert(sequence->events[0].velocity_percent == 100);
  assert(sequence->events[1].velocity_percent == 85);
  assert(sequence->events[2].velocity_percent == 85);
  assert(!ai_keyboard::builtin_music_sequence(99).has_value());

  auto external = *sequence;
  external.event_count = 1;
  external.events[0].octave_offset = -1;
  std::array<std::uint8_t, ai_keyboard::kMusicSequencePayloadLen> payload{};
  assert(!ai_keyboard::encode_music_sequence(external, &payload));
}

void pause_and_resume_keep_the_loaded_song() {
  ai_keyboard::MusicSequence sequence;
  sequence.command = ai_keyboard::MusicSequenceCommand::Play;
  sequence.event_count = 1;
  sequence.events[0] = {0, 4, 100};
  ai_keyboard::MusicSynth synth;
  ai_keyboard::MusicConfig config;
  config.enabled = true;
  assert(synth.apply_config(config));
  ai_keyboard::MusicSequencePlayer player;
  assert(player.load(sequence));
  ai_keyboard::MusicSequence pause;
  pause.command = ai_keyboard::MusicSequenceCommand::Pause;
  assert(player.load(pause) && player.paused());
  player.advance(48000, &synth);
  assert(player.playing());
  ai_keyboard::MusicSequence resume;
  resume.command = ai_keyboard::MusicSequenceCommand::Resume;
  assert(player.load(resume) && !player.paused());
}

void pause_releases_sound_and_resume_preserves_remaining_duration() {
  // 暂停释放当前音符，恢复保持剩余时值而不是重放整拍。
  ai_keyboard::MusicSequence song;
  song.command = ai_keyboard::MusicSequenceCommand::Play;
  song.event_count = 1;
  song.events[0] = {0, 4, 100};
  ai_keyboard::MusicSequencePlayer paused_player;
  ai_keyboard::MusicSynth paused_synth;
  ai_keyboard::MusicConfig pause_config;
  pause_config.enabled = true;
  assert(paused_synth.apply_config(pause_config));
  assert(paused_player.load(song));
  paused_player.advance(480, &paused_synth);
  std::array<std::int16_t, 4800> pause_frame{};
  paused_synth.render(pause_frame.data(), 480);
  ai_keyboard::MusicSequence control;
  control.command = ai_keyboard::MusicSequenceCommand::Pause;
  assert(paused_player.load(control));
  paused_player.advance(pause_frame.size(), &paused_synth);
  paused_synth.render(pause_frame.data(), pause_frame.size());
  assert(!paused_synth.active());
  control.command = ai_keyboard::MusicSequenceCommand::Resume;
  assert(paused_player.load(control));
  paused_player.advance(480, &paused_synth);
  assert(paused_synth.active());
  paused_player.advance(23040, &paused_synth);
  assert(!paused_player.playing());
}

void invalid_internal_sequences_preserve_the_loaded_song() {
  // 内部调用也必须受固定数组容量约束，拒绝后保留原播放器状态。
  ai_keyboard::MusicSequence invalid;
  invalid.command = ai_keyboard::MusicSequenceCommand::Play;
  invalid.event_count = ai_keyboard::kMusicSequenceStorageEvents + 1;
  ai_keyboard::MusicSequencePlayer bounded;
  assert(!bounded.load(invalid));
  invalid.event_count = 1;
  assert(bounded.load(invalid));
  for (const auto bad_event : {
           ai_keyboard::MusicSequenceEvent{8, 4, 100, 0},
           ai_keyboard::MusicSequenceEvent{0, 0, 100, 0},
           ai_keyboard::MusicSequenceEvent{0, 4, 101, 0},
           ai_keyboard::MusicSequenceEvent{0, 4, 100, 2}}) {
    invalid.events[0] = bad_event;
    assert(!bounded.load(invalid));
    assert(bounded.playing());
  }
}

}  // namespace

int main() {
  pause_releases_sound_and_resume_preserves_remaining_duration();
  invalid_internal_sequences_preserve_the_loaded_song();
  protocol_round_trip_and_player_progress();
  builtin_ode_to_joy_is_compact_and_playable();
  pause_and_resume_keep_the_loaded_song();
  return 0;
}
