#include <spdlog/common.h>
#include <map>
#include <regex>

namespace lootcli
{

enum class Progress
{
  None = 0,
  CheckingMasterlistExistence,
  UpdatingMasterlist,
  LoadingLists,
  ReadingPlugins,
  SortingPlugins,
  WritingLoadorder,
  ParsingLootMessages,
  Done
};

enum class MessageType
{
  None = 0,
  Progress,
  Log
};

struct Message
{
  MessageType type = MessageType::None;
  Progress progress = Progress::None;
  spdlog::level::level_enum logLevel = spdlog::level::trace;
  std::string log;

  static Message fromProgress(Progress p)
  {
    return {MessageType::Progress, p, spdlog::level::trace, ""};
  }

  static Message fromLog(spdlog::level::level_enum level, std::string log)
  {
    return {MessageType::Log, Progress::None, level, std::move(log)};
  }
};


inline Message parseMessage(const std::string_view& line)
{
  static std::regex e(R"(^\[([a-z]+)\] (.+)$)");

  std::match_results<std::string_view::const_iterator> m;
  if (!std::regex_match(line.begin(), line.end(), m, e)) {
    return {};
  }

  const auto type = m[1];

  if (type == "progress") {
    try {
      const auto p = std::stoi(m[2]);
      return Message::fromProgress(static_cast<Progress>(p));
    } catch (std::exception&) {
      return {};
    }
  } else {
    const auto level = spdlog::level::from_str(type);
    if (level == spdlog::level::off) {
      return {};
    }

    return Message::fromLog(level, m[2]);
  }
}

} // namespace
