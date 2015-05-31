#ifndef LOOTTHREAD_H
#define LOOTTHREAD_H


#include <string>
#include <boost/filesystem.hpp>
#include <QLibrary>

namespace loot {
  class Game;
}

class LOOTWorker
{
public:
  explicit LOOTWorker();

  void setGame(const std::string &gameName);
  void setGamePath(const std::string &gamePath);
  void setOutput(const std::string &outputPath);

  void setUpdateMasterlist(bool update);

  void run();

private:

  void progress(const std::string &step);
  void errorOccured(const std::string &message);

  boost::filesystem::path masterlistPath();
  boost::filesystem::path userlistPath();
  const char *repoUrl();

private:

  void handleErr(unsigned int resultCode, const char *description);
  bool sort(loot::Game &game);
  const char *lootErrorString(unsigned int errorCode);
  template <typename T> T resolveVariable(QLibrary &lib, const char *name);
  template <typename T> T resolveFunction(QLibrary &lib, const char *name);

private:

  int m_GameId;
  int m_Language;
  std::string m_GameName;
  std::string m_GamePath;
  std::string m_OutputPath;
  bool m_UpdateMasterlist;
  QLibrary m_Library;

  std::map<std::string, QFunctionPointer> m_ResolveLookup;

};

#endif // LOOTTHREAD_H
