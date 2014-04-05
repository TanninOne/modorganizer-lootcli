#ifndef LOOTTHREAD_H
#define LOOTTHREAD_H


#include <string>

namespace loot {
  class Game;
}

class LOOTWorker
{
public:
  explicit LOOTWorker();

  void setGame(const std::string &gameName);
  void setGamePath(const std::string &gamePath);
  void setMasterlist(const std::string &masterlistPath);
  void setUserlist(const std::string &userlistPath);
  void setOutput(const std::string &outputPath);

  void setUpdateMasterlist(bool update);

  void run();

private:

  void progress(const std::string &step);
  void errorOccured(const std::string &message);

private:

  void handleErr(unsigned int resultCode, const char *description);
  void sort(loot::Game &game);

private:

  int m_GameId;
  int m_Language;
  std::string m_GamePath;
  std::string m_MasterlistPath;
  std::string m_UserlistPath;
  std::string m_OutputPath;
  bool m_UpdateMasterlist;
};

#endif // LOOTTHREAD_H
