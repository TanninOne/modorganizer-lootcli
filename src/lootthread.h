#ifndef LOOTTHREAD_H
#define LOOTTHREAD_H

#include <lootcli/lootcli.h>
#include "game_settings.h"

namespace loot {
  class Game;
}


namespace lootcli {

loot::LogLevel toLootLogLevel(lootcli::LogLevels level);
lootcli::LogLevels fromLootLogLevel(loot::LogLevel level);

class LOOTWorker
{
public:
  explicit LOOTWorker();

  void setGame(const std::string &gameName);
  void setGamePath(const std::string &gamePath);
  void setOutput(const std::string &outputPath);
  void setPluginListPath(const std::string &pluginListPath);
  void setLanguageCode(const std::string &language_code); //Will add this when I figure out how languages work on MO
  void setLogLevel(loot::LogLevel level);

  void setUpdateMasterlist(bool update);
  std::string formatDirty(const loot::PluginCleaningData &cleaningData);
  loot::GameSettings m_GameSettings;

  int run();

private:
  void progress(Progress p);
  void log(loot::LogLevel level, const std::string& message);

  void getSettings(const std::filesystem::path& file);

  std::filesystem::path masterlistPath();
  std::filesystem::path settingsPath();
  std::filesystem::path userlistPath();
  std::filesystem::path l10nPath();
  std::filesystem::path dataPath();

private:

 // void handleErr(unsigned int resultCode, const char *description);
  bool sort(loot::Game &game);
  //const char *lootErrorString(unsigned int errorCode);
  //template <typename T> T resolveVariable(HMODULE lib, const char *name);
  //template <typename T> T resolveFunction(HMODULE lib, const char *name);

private:
  loot::GameType m_GameId;
  std::string m_Language;
  std::string m_GameName;
  std::string m_GamePath;
  std::string m_OutputPath;
  std::string m_PluginListPath;
  loot::LogLevel m_LogLevel;
  bool m_UpdateMasterlist;
  //HMODULE m_Library;

  //std::map<std::string, FARPROC> m_ResolveLookup;
  mutable std::recursive_mutex mutex_;
};

} // namespace

#endif // LOOTTHREAD_H
