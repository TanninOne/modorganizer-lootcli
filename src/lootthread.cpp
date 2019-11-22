#include "lootthread.h"

#pragma warning (push, 0)
#include <loot/api.h>
#pragma warning (pop)

#include <thread>
#include <mutex>
#include <ctype.h>
#include <map>
#include <exception>
#include <stdio.h>
#include <stddef.h>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <sstream>
#include <fstream>
#include <memory>
#include <filesystem>

#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#include <game_settings.h>

#pragma warning(push)
#pragma warning(disable: 4003)
#include <cpptoml.h>
#pragma warning(pop)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>

//using namespace loot;
namespace fs = std::filesystem;

using std::lock_guard;
using std::recursive_mutex;

namespace lootcli
{

fs::path utf8_path(const std::string& s)
{
  std::u8string u8s;
  u8s.reserve(s.size());

  for (const char c : s) {
    u8s.append(1 ,static_cast<const char8_t>(c));
  }

  return u8s;
}


LOOTWorker::LOOTWorker()
    : m_GameId(loot::GameType::tes5)
    , m_Language(loot::MessageContent::defaultLanguage)
    , m_GameName("Skyrim")
    , m_LogLevel(loot::LogLevel::info)
{
}

std::string ToLower(const std::string& text)
{
    std::string result = text;
    std::transform(text.begin(), text.end(), result.begin(), tolower);
    return result;
}


void LOOTWorker::setGame(const std::string& gameName)
{
  static std::map<std::string, loot::GameType> gameMap = {
    {"morrowind",  loot::GameType::tes3},
    {"oblivion",   loot::GameType::tes4},
    {"fallout3",   loot::GameType::fo3},
    {"fallout4",   loot::GameType::fo4},
    {"fallout4vr", loot::GameType::fo4vr},
    {"falloutnv",  loot::GameType::fonv},
    {"skyrim",     loot::GameType::tes5},
    {"skyrimse",   loot::GameType::tes5se},
    {"skyrimvr",   loot::GameType::tes5vr},
  };

    auto iter = gameMap.find(ToLower(gameName));

    if (iter != gameMap.end()) {
        m_GameName = gameName;
        if (ToLower(gameName) == "skyrimse") {
            m_GameName = "Skyrim Special Edition";
        }
        m_GameId = iter->second;
    }
    else {
        throw std::runtime_error((boost::format("invalid game name \"%1%\"") % gameName).str());
    }
}

void LOOTWorker::setGamePath(const std::string& gamePath)
{
    m_GamePath = gamePath;
}

void LOOTWorker::setOutput(const std::string& outputPath)
{
    m_OutputPath = outputPath;
}

void LOOTWorker::setUpdateMasterlist(bool update)
{
    m_UpdateMasterlist = update;
}

void LOOTWorker::setPluginListPath(const std::string& pluginListPath)
{
    m_PluginListPath = pluginListPath;
}

void LOOTWorker::setLanguageCode(const std::string& languageCode)
{
    m_Language = languageCode;
}

void LOOTWorker::setLogLevel(loot::LogLevel level)
{
  m_LogLevel = level;
}

fs::path GetLOOTAppData() {
    TCHAR path[MAX_PATH];

    HRESULT res = ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path);

    if (res == S_OK) {
        return fs::path(path) / "LOOT";
    } else {
        return fs::path("");
    }
}

fs::path LOOTWorker::masterlistPath() {
    return GetLOOTAppData() / m_GameSettings.FolderName() / "masterlist.yaml";
}

fs::path LOOTWorker::userlistPath() {
    return GetLOOTAppData() / m_GameSettings.FolderName() / "userlist.yaml";
}
fs::path LOOTWorker::settingsPath()
{
    return GetLOOTAppData() / "settings.toml";
}

fs::path LOOTWorker::l10nPath()
{
    return GetLOOTAppData() / "resources" / "l10n";
}

fs::path LOOTWorker::dataPath()
{
    return fs::path(m_GamePath) / m_GameSettings.DataPath();
}

std::string LOOTWorker::formatDirty(const loot::PluginCleaningData& cleaningData) {

    const std::string itmRecords = std::to_string(cleaningData.GetITMCount()) + " ITM record(s)";
    const std::string deletedReferences = std::to_string(cleaningData.GetDeletedReferenceCount()) + " deleted reference(s)";
    const std::string deletedNavmeshes = std::to_string(cleaningData.GetDeletedNavmeshCount()) + " deleted navmesh(es)";
    std::string message;
    if (cleaningData.GetITMCount() > 0 && cleaningData.GetDeletedReferenceCount() > 0 && cleaningData.GetDeletedNavmeshCount() > 0)
        message = cleaningData.GetCleaningUtility() + " found " + itmRecords + ", " + deletedReferences + " and " + deletedNavmeshes + ".";
    else if (cleaningData.GetITMCount() == 0 && cleaningData.GetDeletedReferenceCount() == 0 && cleaningData.GetDeletedNavmeshCount() == 0)
        message = cleaningData.GetCleaningUtility() + " found dirty edits.";
    else if (cleaningData.GetITMCount() == 0 && cleaningData.GetDeletedReferenceCount() > 0 && cleaningData.GetDeletedNavmeshCount() > 0)
        message = cleaningData.GetCleaningUtility() + " found " + deletedReferences + " and " + deletedNavmeshes + ".";
    else if (cleaningData.GetITMCount() > 0 && cleaningData.GetDeletedReferenceCount() == 0 && cleaningData.GetDeletedNavmeshCount() > 0)
        message = cleaningData.GetCleaningUtility() + " found " + itmRecords + " and " + deletedNavmeshes + ".";
    else if (cleaningData.GetITMCount() > 0 && cleaningData.GetDeletedReferenceCount() > 0 && cleaningData.GetDeletedNavmeshCount() == 0)
        message = cleaningData.GetCleaningUtility() + " found " + itmRecords + " and " + deletedReferences + ".";
    else if (cleaningData.GetITMCount() > 0)
        message = cleaningData.GetCleaningUtility() + " found " + itmRecords + ".";
    else if (cleaningData.GetDeletedReferenceCount() > 0)
        message = cleaningData.GetCleaningUtility() + " found " + deletedReferences + ".";
    else if (cleaningData.GetDeletedNavmeshCount() > 0)
        message = cleaningData.GetCleaningUtility() + " found " + deletedNavmeshes + ".";

    if (cleaningData.GetInfo().empty()) {
        return loot::Message(loot::MessageType::warn, message).ToSimpleMessage(m_Language).text;
    }

    auto info = cleaningData.GetInfo();
    for (auto& content : info) {
        content = loot::MessageContent(message + " " + content.GetText(), content.GetLanguage());
    }

    return loot::Message(loot::MessageType::warn, info).ToSimpleMessage(m_Language).text;
}

void LOOTWorker::getSettings(const fs::path& file) {
    lock_guard<recursive_mutex> guard(mutex_);

    auto settings = cpptoml::parse_file(file.string());
    auto games = settings->get_table_array("games");
    if (games) {
        for (const auto& game : *games) {
            try {
                using loot::GameSettings;
                using loot::GameType;

                GameSettings newSettings;

                auto type = game->get_as<std::string>("type");
                if (!type) {
                    throw std::runtime_error("'type' key missing from game settings table");
                }

                auto folder = game->get_as<std::string>("folder");
                if (!folder) {
                    throw std::runtime_error("'folder' key missing from game settings table");
                }

                if (*type == GameSettings(GameType::tes3).FolderName()) {
                    newSettings = GameSettings(GameType::tes3, *folder);
                } else if (*type == GameSettings(GameType::tes4).FolderName()) {
                    newSettings = GameSettings(GameType::tes4, *folder);
                } else if (*type == GameSettings(GameType::tes5).FolderName()) {
                    newSettings = GameSettings(GameType::tes5, *folder);
                } else if (*type == GameSettings(GameType::tes5se).FolderName()) {
                    newSettings = GameSettings(GameType::tes5se, *folder);
                } else if (*type == GameSettings(GameType::tes5vr).FolderName()) {
                    newSettings = GameSettings(GameType::tes5vr, *folder);
                } else if (*type == GameSettings(GameType::fo3).FolderName()) {
                    newSettings = GameSettings(GameType::fo3, *folder);
                } else if (*type == GameSettings(GameType::fonv).FolderName()) {
                    newSettings = GameSettings(GameType::fonv, *folder);
                } else if (*type == GameSettings(GameType::fo4).FolderName()) {
                    newSettings = GameSettings(GameType::fo4, *folder);
                } else if (*type == GameSettings(GameType::fo4vr).FolderName()) {
                    newSettings = GameSettings(GameType::fo4vr, *folder);
                } else
                    throw std::runtime_error(
                        "invalid value for 'type' key in game settings table");

                if (newSettings.Type() == m_GameSettings.Type()) {

                    auto name = game->get_as<std::string>("name");
                    if (name) {
                        newSettings.SetName(*name);
                    }

                    auto master = game->get_as<std::string>("master");
                    if (master) {
                        newSettings.SetMaster(*master);
                    }

                    auto minimumHeaderVersion = game->get_as<double>("minimumHeaderVersion");
                    if (minimumHeaderVersion) {
                        newSettings.SetMinimumHeaderVersion((float)* minimumHeaderVersion);
                    }

                    auto repo = game->get_as<std::string>("repo");
                    if (repo) {
                        newSettings.SetRepoURL(*repo);
                    }

                    auto branch = game->get_as<std::string>("branch");
                    if (branch) {
                        newSettings.SetRepoBranch(*branch);

                        auto defaultGame = GameSettings(newSettings.Type());
                        if (newSettings.RepoURL() == defaultGame.RepoURL() &&
                            newSettings.IsRepoBranchOldDefault()) {
                            newSettings.SetRepoBranch(defaultGame.RepoBranch());
                        }
                    }

                    auto path = game->get_as<std::string>("path");
                    if (path) {
                        newSettings.SetGamePath(utf8_path(*path));
                    }

                    auto localPath = game->get_as<std::string>("local_path");
                    if (localPath) {
                        newSettings.SetGameLocalPath(utf8_path(*localPath));
                    }

                    auto registry = game->get_as<std::string>("registry");
                    if (registry) {
                        newSettings.SetRegistryKey(*registry);
                    }

                    m_GameSettings = newSettings;
                    break;
                }
            }
            catch (...) {
                // Skip invalid games.
            }
        }
    }

    m_Language = settings->get_as<std::string>("language").value_or(m_Language);

    if (m_Language != loot::MessageContent::defaultLanguage) {
      log(loot::LogLevel::debug, "initialising language settings");
      log(loot::LogLevel::debug, "selected language: " + m_Language);

      //Boost.Locale initialisation: Generate and imbue locales.
      boost::locale::generator gen;
      std::locale::global(gen(m_Language + ".UTF-8"));
      loot::InitialiseLocale(m_Language + ".UTF-8");
    }
}

std::string escape(const std::string& s)
{
  return boost::replace_all_copy(s, "\"", "\\\"");
}

int LOOTWorker::run()
{
    // Do some preliminary locale / UTF-8 support setup here, in case the settings file reading requires it.
    //Boost.Locale initialisation: Specify location of language dictionaries.
    boost::locale::generator gen;
    gen.add_messages_path(l10nPath().string());
    gen.add_messages_domain("loot");

    //Boost.Locale initialisation: Generate and imbue locales.
    std::locale::global(gen("en.UTF-8"));
    loot::InitialiseLocale("en.UTF-8");

    loot::SetLoggingCallback([&](loot::LogLevel level, const char* message) {
      log(level, message);
    });


    try {
        // ensure the loot directory exists
        fs::path lootAppData = GetLOOTAppData();
        if (lootAppData.empty()) {
            log(loot::LogLevel::error, "failed to create loot app data path");
            return 1;
        }

        if (!fs::exists(lootAppData)) {
            fs::create_directory(lootAppData);
        }

        fs::path profile(m_PluginListPath);
        profile = profile.parent_path();
        auto gameHandle = CreateGameHandle(
          m_GameId, utf8_path(m_GamePath), utf8_path(profile.string()));
        auto db = gameHandle->GetDatabase();

        m_GameSettings = loot::GameSettings(m_GameId);

        fs::path settings = settingsPath();

        if (fs::exists(settings))
            getSettings(settings);

        bool mlUpdated = false;
        if (m_UpdateMasterlist) {
            progress(Progress::CheckingMasterlistExistence);
            if (!fs::exists(masterlistPath())) {
                fs::create_directories(masterlistPath().parent_path());
            }

            progress(Progress::UpdatingMasterlist);

            mlUpdated = db->UpdateMasterlist(
              utf8_path(masterlistPath().string()),
              m_GameSettings.RepoURL(),
              m_GameSettings.RepoBranch());

            if (mlUpdated && !db->IsLatestMasterlist(utf8_path(masterlistPath().string()), m_GameSettings.RepoBranch())) {
                log(loot::LogLevel::error,
                  "the latest masterlist revision contains a syntax error, "
                  "LOOT is using the most recent valid revision instead. "
                  "Syntax errors are usually minor and fixed within hours.");
            }
        }

        progress(Progress::LoadingLists);

        fs::path userlist = userlistPath();
        db->LoadLists(
          utf8_path(masterlistPath().string()),
          fs::exists(userlist) ? utf8_path(userlistPath().string()) : fs::path());

        progress(Progress::ReadingPlugins);

        std::vector<std::string> pluginsList;
        for (fs::directory_iterator it(dataPath()); it != fs::directory_iterator(); ++it) {
            if (fs::is_regular_file(it->status()) && gameHandle->IsValidPlugin(it->path().filename().string())) {
                std::string name = it->path().filename().string();
                log(loot::LogLevel::info, "found plugin: " + name);
                pluginsList.push_back(name);
            }
        }

        progress(Progress::SortingPlugins);

        gameHandle->LoadCurrentLoadOrderState();
        std::vector<std::string> sortedPlugins = gameHandle->SortPlugins(pluginsList);

        progress(Progress::WritingLoadorder);

        std::ofstream outf(utf8_path(m_PluginListPath));
        if (!outf) {
            log(loot::LogLevel::error, "failed to open " + m_PluginListPath + " to rewrite it");
            return 1;
        }
        outf << "# This file was automatically generated by Mod Organizer." << std::endl;
        for (const std::string& plugin : sortedPlugins) {
            outf << plugin << std::endl;
        }
        outf.close();


        progress(Progress::ParsingLootMessages);

        // this builds the json manually because Qt was removed as a dependency
        // and boost doesn't have anything for arbitrary json (property_tree
        // doesn't support arrays)
        //
        // this can be cleaned up the day there's a json library in the
        // dependencies


        std::ofstream buf;
        buf.open(utf8_path(m_OutputPath).c_str());

        buf << "[\n";

        for (size_t i = 0; i < sortedPlugins.size(); ++i) {
          if (i > 0) {
            buf << ",\n";
          }

          buf << "{\n";

          buf << "\t\"name\": \"" << escape(sortedPlugins[i]) << "\"";

          std::vector<loot::Message> pluginMessages;
          pluginMessages = db->GetGeneralMessages(true);

          if (!pluginMessages.empty()) {
            buf
              << ",\n\t\"messages\":\n"
              << "\t[\n";

            bool first = true;
            for (loot::Message message : pluginMessages) {
              if (!first) {
                buf << ",\n";
              }

              const char* type;
              if (message.GetType() == loot::MessageType::say) {
                  type = "info";
              } else if (message.GetType() == loot::MessageType::warn) {
                  type = "warn";
              } else if (message.GetType() == loot::MessageType::error) {
                  type = "error";
              } else {
                  type = "unknown";

                  log(
                    loot::LogLevel::error,
                    "invalid message type " + std::to_string(static_cast<int>(message.GetType())));
              }

              buf
                << "\t\t{\n"
                << "\t\t\"type\": \"" << escape(type) << "\",\n"
                << "\t\t\"message\": \"" << escape(message.ToSimpleMessage(m_Language).text) << "\"\n"
                << "\t\t}";

              first = false;
            }

            buf << "\n\t]";
          }

          auto metaData = db->GetPluginMetadata(sortedPlugins[i], true, true);
          if (metaData) {
            const std::set<loot::PluginCleaningData> dirtyInfo = metaData->GetDirtyInfo();

            if (!dirtyInfo.empty()) {
              buf
                << ",\n"
                << "\t\"dirty\":\n"
                << "\t[\n";

              bool first = true;
              for (const auto& element : dirtyInfo) {
                if (!first) {
                  buf << ",\n";
                }

                buf << "\t\t\"" + escape(formatDirty(element)) + "\"\n";
                first = false;
              }

              buf << "\n\t]";
            }
          }

          buf << "}";
        }

        buf << "\n\t]";
    }
    catch (const std::exception & e) {
        log(loot::LogLevel::error, std::string("LOOT failed: ") + e.what());
        return 1;
    }

    progress(Progress::Done);

    return 0;
}

void LOOTWorker::progress(Progress p)
{
  std::cout << "[progress] " << static_cast<int>(p) << "\n";
  std::cout.flush();
}

std::string escapeNewlines(const std::string& s)
{
  auto ss = boost::replace_all_copy(s, "\n", "\\n");
  boost::replace_all(ss, "\r", "\\r");
  return ss;
}

void LOOTWorker::log(loot::LogLevel level, const std::string& message)
{
  if (level < m_LogLevel) {
    return;
  }

  const auto ll = fromLootLogLevel(level);
  const auto levelName = logLevelToString(ll);

  std::cout << "[" << levelName << "] " << escapeNewlines(message) << "\n";
  std::cout.flush();
}


loot::LogLevel toLootLogLevel(lootcli::LogLevels level)
{
  using L = loot::LogLevel;
  using LC = lootcli::LogLevels;

  switch (level)
  {
    case LC::Trace:   return L::trace;
    case LC::Debug:   return L::debug;
    case LC::Info:    return L::info;
    case LC::Warning: return L::warning;
    case LC::Error:   return L::error;
    default:          return L::info;
  }
}

lootcli::LogLevels fromLootLogLevel(loot::LogLevel level)
{
  using L = loot::LogLevel;
  using LC = lootcli::LogLevels;

  switch (level)
  {
    case L::trace:
      return LC::Trace;

    case L::debug:
      return LC::Debug;

    case L::info:
      return LC::Info;

    case L::warning:
      return LC::Warning;

    case L::error:  // fall-through
    case L::fatal:
      return LC::Error;

    default:
      return LC::Info;
  }
}

} // namespace
