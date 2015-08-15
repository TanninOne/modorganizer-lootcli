#include "lootthread.h"
#include "../uibase/scopeguard.h"
#pragma warning (push, 0)
#include <loot/api.h>
#pragma warning (pop)
#include <map>
#include <list>
#include <boost/assign.hpp>
#include <boost/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QLibrary>

using namespace std;
using namespace loot;
namespace fs = boost::filesystem;



LOOTWorker::LOOTWorker()
  : m_GameId(0)
  , m_Language(0)
  , m_GameName("Skyrim")
  , m_Library("loot32.dll")
{
}


#define LVAR(variable) resolveVariable<decltype(variable)>(m_Library, #variable)
#define LFUNC(func) resolveFunction<decltype(&func)>(m_Library, #func)


std::string ToLower(const std::string &text)
{
  std::string result = text;
  std::transform(text.begin(), text.end(), result.begin(), tolower);
  return result;
}


const char *LOOTWorker::lootErrorString(unsigned int errorCode)
{
  // can't use switch-case here because the loot_error constants are externs
  if (errorCode == LVAR(loot_error_liblo_error))         return "There was an error in performing a load order operation.";
  if (errorCode == LVAR(loot_error_file_write_fail))     return "A file could not be written to.";
  if (errorCode == LVAR(loot_error_parse_fail))          return "There was an error parsing the file.";
  if (errorCode == LVAR(loot_error_condition_eval_fail)) return "There was an error evaluating the conditionals in a metadata file.";
  if (errorCode == LVAR(loot_error_regex_eval_fail))     return "There was an error evaluating the regular expressions in a metadata file.";
  if (errorCode == LVAR(loot_error_no_mem))              return "The API was unable to allocate the required memory.";
  if (errorCode == LVAR(loot_error_invalid_args))        return "Invalid arguments were given for the function.";
  if (errorCode == LVAR(loot_error_no_tag_map))          return "No Bash Tag map has been generated yet.";
  if (errorCode == LVAR(loot_error_path_not_found))      return "A file or folder path could not be found.";
  if (errorCode == LVAR(loot_error_no_game_detected))    return "The given game could not be found.";
  if (errorCode == LVAR(loot_error_windows_error))       return "An error occurred during a call to the Windows API.";
  if (errorCode == LVAR(loot_error_sorting_error))       return "An error occurred while sorting plugins.";
  return "Unknown error code";
}



template <typename T> T LOOTWorker::resolveVariable(QLibrary &lib, const char *name)
{
  auto iter = m_ResolveLookup.find(name);
  QFunctionPointer ptr;
  if (iter == m_ResolveLookup.end()) {
    ptr = lib.resolve(name);
    m_ResolveLookup.insert(std::make_pair(std::string(name), ptr));
  } else {
    ptr = iter->second;
  }

  if (ptr == NULL) {
    throw std::runtime_error(QObject::tr("invalid dll: %1").arg(lib.errorString()).toLatin1().constData());
  }

  return *reinterpret_cast<T*>(ptr);
}


template <typename T> T LOOTWorker::resolveFunction(QLibrary &lib, const char *name)
{
  auto iter = m_ResolveLookup.find(name);
  QFunctionPointer ptr;
  if (iter == m_ResolveLookup.end()) {
    ptr = lib.resolve(name);
    m_ResolveLookup.insert(std::make_pair(std::string(name), ptr));
  } else {
    ptr = iter->second;
  }

  if (ptr == NULL) {
    throw std::runtime_error(QObject::tr("invalid dll: %1").arg(lib.errorString()).toLatin1().constData());
  }

  return reinterpret_cast<T>(ptr);
}


void LOOTWorker::setGame(const std::string &gameName)
{
  static std::map<std::string, unsigned int> gameMap = boost::assign::map_list_of
    ( "oblivion", LVAR(loot_game_tes4) )
    ( "fallout3", LVAR(loot_game_fo3) )
    ( "falloutnv", LVAR(loot_game_fonv) )
    ( "skyrim", LVAR(loot_game_tes5) );

  auto iter = gameMap.find(ToLower(gameName));
  if (iter != gameMap.end()) {
    m_GameName = gameName;
    m_GameId = iter->second;
  } else {
    throw std::runtime_error((boost::format("invalid game name \"%1%\"") % gameName).str());
  }
}

void LOOTWorker::setGamePath(const std::string &gamePath)
{
  m_GamePath = gamePath;
}

void LOOTWorker::setOutput(const std::string &outputPath)
{
  m_OutputPath = outputPath;
}

void LOOTWorker::setUpdateMasterlist(bool update)
{
  m_UpdateMasterlist = update;
}

void LOOTWorker::handleErr(unsigned int resultCode, const char *description)
{
  if (resultCode != LVAR(loot_ok)) {
    const char *errMessage;
    unsigned int lastError = LFUNC(loot_get_error_message)(&errMessage);
    throw std::runtime_error((boost::format("%1% failed: %2% (code %3%)") % description % errMessage % lastError).str());
  } else {
    progress(description);
  }
}


boost::filesystem::path GetLOOTAppData() {
  TCHAR path[MAX_PATH];

  HRESULT res = ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, path);

  if (res == S_OK) {
    return fs::path(path) / "LOOT";
  } else {
    return fs::path("");
  }
}


boost::filesystem::path LOOTWorker::masterlistPath()
{
  return GetLOOTAppData() / m_GameName / "masterlist.yaml";
}

boost::filesystem::path LOOTWorker::userlistPath()
{
  return GetLOOTAppData() / m_GameName / "userlist.yaml";
}

const char *LOOTWorker::repoUrl()
{
  if (m_GameId == LVAR(loot_game_tes4))
    return "https://github.com/loot/oblivion.git";
  else if (m_GameId == LVAR(loot_game_tes5))
    return "https://github.com/loot/skyrim.git";
  else if (m_GameId == LVAR(loot_game_fo3))
    return "https://github.com/loot/fallout3.git";
  else if (m_GameId == LVAR(loot_game_fonv))
    return "https://github.com/loot/falloutnv.git";
  else
    return "";
}


void LOOTWorker::run()
{
  try {
    // ensure the loot directory exists
    fs::path lootAppData = GetLOOTAppData();
    if (lootAppData.empty()) {
      errorOccured("failed to create loot app data path");
      return;
    }
    if (!fs::exists(lootAppData)) {
      fs::create_directory(lootAppData);
    }

    loot_db db;

    unsigned int res = LFUNC(loot_create_db)(&db, m_GameId, m_GamePath.c_str(), nullptr);
    if (res != LVAR(loot_ok)) {
      errorOccured((boost::format("failed to create db: %1%") % lootErrorString(res)).str());
      return;
    }

    ON_BLOCK_EXIT([&] () {
      LFUNC(loot_destroy_db)(db);
      LFUNC(loot_cleanup)();
    });

    bool mlUpdated = false;
    if (m_UpdateMasterlist) {
      progress("updating masterlist");
      if (!fs::exists(masterlistPath())) {
        fs::create_directories(masterlistPath());
      }
      unsigned int res = LFUNC(loot_update_masterlist)(db
                                                      , masterlistPath().string().c_str()
                                                      , repoUrl()
                                                      , "master"
                                                      , &mlUpdated);
      if (res != LVAR(loot_ok)) {
        progress((boost::format("failed to update masterlist: %1%") % lootErrorString(res)).str());
      }
    }

    fs::path userlist = userlistPath();


    res = LFUNC(loot_load_lists)(db
                                 , masterlistPath().string().c_str()
                                 , fs::exists(userlist) ? userlistPath().string().c_str()
                                                        : nullptr);
    if (res != LVAR(loot_ok)) {
      progress((boost::format("failed to load lists: %1%") % lootErrorString(res)).str());
    }

    res = LFUNC(loot_eval_lists)(db, LVAR(loot_lang_any));
    if (res != LVAR(loot_ok)) {
      progress((boost::format("failed to evaluate lists: %1%") % lootErrorString(res)).str());
    }

    char **sortedPlugins;
    size_t numPlugins = 0;

    progress("sorting plugins");
    res = LFUNC(loot_sort_plugins)(db, &sortedPlugins, &numPlugins);
    if (res != LVAR(loot_ok)) {
      progress((boost::format("failed to sort plugins: %1%") % lootErrorString(res)).str());
    }

    progress("applying load order");
    res = LFUNC(loot_apply_load_order)(db, sortedPlugins, numPlugins);
    if (res != LVAR(loot_ok)) {
      progress((boost::format("failed to apply load order: %1%") % lootErrorString(res)).str());
    }

    QJsonArray report;

    progress("retrieving loot messages");
    for (size_t i = 0; i < numPlugins; ++i) {
      QJsonObject modInfo;
      modInfo.insert("name", sortedPlugins[i]);
      loot_message *pluginMessages = nullptr;
      size_t numMessages = 0;
      res = LFUNC(loot_get_plugin_messages)(db, sortedPlugins[i], &pluginMessages, &numMessages);
      if (res != LVAR(loot_ok)) {
        progress((boost::format("failed to retrieve plugin messages: %1%") % lootErrorString(res)).str());
      }
      QJsonArray messages;
      if (pluginMessages != nullptr) {
        for (size_t j = 0; j < numMessages; ++j) {
          QJsonObject message;
          QString type;
          if (pluginMessages[j].type == LVAR(loot_message_say))
            type = "info";
          else if (pluginMessages[j].type == LVAR(loot_message_warn))
            type = "warn";
          else if (pluginMessages[j].type == LVAR(loot_message_error))
            type = "error";
          else {
            errorOccured((boost::format("invalid message type %1%") % pluginMessages[j].type).str());
            type = "unknown";
          }

          message.insert("type", type);
          message.insert("message", QJsonValue(pluginMessages[j].message));
          messages.append(message);
        }
      }

      modInfo.insert("messages", messages);

      unsigned int dirtyState = 0;
      res = LFUNC(loot_get_dirty_info)(db, sortedPlugins[i], &dirtyState);
      if (res != LVAR(loot_ok)) {
        progress((boost::format("failed to retrieve plugin messages: %1%") % lootErrorString(res)).str());
      } else {
        QString dirty = dirtyState == LVAR(loot_needs_cleaning_yes) ? "yes"
                      : dirtyState == LVAR(loot_needs_cleaning_no)  ? "no"
                      : "unknown";
        modInfo.insert("dirty", dirty);
      }
      report.append(modInfo);
    }

    QJsonDocument doc(report);
    QFile output(m_OutputPath.c_str());
    output.open(QIODevice::WriteOnly);
    output.write(doc.toJson());
    output.close();
  } catch (const std::exception &e) {
    errorOccured((boost::format("LOOT failed: %1%") % e.what()).str());
  }
  progress("done");
}

void LOOTWorker::progress(const string &step)
{
  BOOST_LOG_TRIVIAL(info) << "[progress] " << step;
  fflush(stdout);
}

void LOOTWorker::errorOccured(const string &message)
{
  BOOST_LOG_TRIVIAL(error) << message;
  fflush(stdout);
}
