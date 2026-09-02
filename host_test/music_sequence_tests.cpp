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
  assert(sequence->event_count == 98);
  assert(sequence->event_count <= ai_keyboard::kMusicSequenceStorageEvents);
  assert(sequence->events[0].degree == 2);
  assert(sequence->events[12].duration_sixteenths == 6);
  assert(sequence->events[14].duration_sixteenths == 8);
  assert(sequence->events[15].degree == 2);
  assert(sequence->events[29].duration_sixteenths == 8);
  assert(sequence->events[30].degree == 1);
  assert(sequence->events[47].degree == 2);
  assert(sequence->events[97].duration_sixteenths == 8);
  assert(sequence->events[0].velocity_percent == 100);
  assert(sequence->events[1].velocity_percent == 85);
  assert(sequence->events[2].velocity_percent == 85);
  assert(!ai_keyboard::builtin_music_sequence(99).has_value());
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

}  // namespace

int main() {
  protocol_round_trip_and_player_progress();
  builtin_ode_to_joy_is_compact_and_playable();
  pause_and_resume_keep_the_loaded_song();
  return 0;
}
