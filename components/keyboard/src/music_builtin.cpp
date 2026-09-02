#include "keyboard/music_builtin.h"

namespace ai_keyboard {
namespace {

constexpr std::array<MusicSequenceEvent, 15> kPhraseA{{
    {2, 4, 0}, {2, 4, 0}, {3, 4, 0}, {4, 4, 0},
    {4, 4, 0}, {3, 4, 0}, {2, 4, 0}, {1, 4, 0},
    {0, 4, 0}, {0, 4, 0}, {1, 4, 0}, {2, 4, 0},
    {2, 6, 0}, {1, 2, 0}, {1, 8, 0}}};
constexpr std::array<MusicSequenceEvent, 15> kPhraseB{{
    {2, 4, 0}, {2, 4, 0}, {3, 4, 0}, {4, 4, 0},
    {4, 4, 0}, {3, 4, 0}, {2, 4, 0}, {1, 4, 0},
    {0, 4, 0}, {0, 4, 0}, {1, 4, 0}, {2, 4, 0},
    {1, 6, 0}, {0, 2, 0}, {0, 8, 0}}};
constexpr std::array<MusicSequenceEvent, 18> kPhraseC{{
    {1, 4, 0}, {1, 4, 0}, {2, 4, 0}, {0, 4, 0},
    {1, 4, 0}, {2, 2, 0}, {3, 2, 0}, {2, 4, 0}, {0, 4, 0},
    {1, 4, 0}, {2, 2, 0}, {3, 2, 0}, {2, 4, 0}, {1, 4, 0},
    {0, 4, 0}, {1, 4, 0}, {4, 4, 0, -1}, {2, 4, 0}}};
constexpr std::array<MusicSequenceEvent, 15> kPhraseD{{
    {2, 4, 0}, {2, 4, 0}, {3, 4, 0}, {4, 4, 0},
    {4, 4, 0}, {3, 4, 0}, {2, 4, 0}, {1, 4, 0},
    {0, 4, 0}, {0, 4, 0}, {1, 4, 0}, {2, 4, 0},
    {1, 6, 0}, {0, 2, 0}, {0, 8, 0}}};
constexpr std::size_t kBuiltinEventCount =
    kPhraseA.size() + kPhraseB.size() + 2U * (kPhraseC.size() + kPhraseD.size());

}  // namespace

std::optional<MusicSequence> builtin_music_sequence(std::uint8_t builtin_id) {
  if (builtin_id != kBuiltinSongOdeToJoy) return std::nullopt;

  // 只存图片上方主旋律；下方伴奏不写入，频率仍由 MusicSynth 实时计算。
  std::array<MusicSequenceEvent, kBuiltinEventCount> events{};
  std::size_t cursor = 0;
  const auto append = [&](const auto& phrase) {
    for (const auto& event : phrase) events[cursor++] = event;
  };
  append(kPhraseA);
  append(kPhraseB);
  append(kPhraseC);
  append(kPhraseD);
  append(kPhraseC);
  append(kPhraseD);

  MusicSequence sequence;
  sequence.command = MusicSequenceCommand::Play;
  sequence.bpm = 96;
  sequence.event_count = static_cast<std::uint8_t>(events.size());
  std::uint16_t sixteenth_cursor = 0;
  for (std::size_t index = 0; index < sequence.event_count; ++index) {
    // 4/4 的第一拍最强，其它正拍中等，弱拍较轻；无需逐音符维护力度。
    const auto beat_position = sixteenth_cursor % 16U;
    const auto velocity = static_cast<std::uint8_t>(
        beat_position == 0U ? 100U : (beat_position % 4U == 0U ? 85U : 70U));
    sequence.events[index] = {events[index].degree,
                              events[index].duration_sixteenths,
                              velocity,
                              events[index].octave_offset};
    sixteenth_cursor = static_cast<std::uint16_t>(
        sixteenth_cursor + events[index].duration_sixteenths);
  }
  return sequence;
}

}  // namespace ai_keyboard
