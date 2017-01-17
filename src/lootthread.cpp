#include "lootthread.h"
#pragma warning (push, 0)

//#include <loot/api.h>

#pragma warning (pop)

#include <boost/assign.hpp>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>

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

using namespace loot;
namespace fs = boost::filesystem;

using boost::property_tree::ptree;
using boost::property_tree::write_json;


LOOTWorker::LOOTWorker()
  : m_GameId(GameType::tes5)
  , m_Language(LanguageCode::english)
  , m_GameName("Skyrim")
{
}


std::string ToLower(const std::string &text)
{
  std::string result = text;
  std::transform(text.begin(), text.end(), result.begin(), tolower);
  return result;
}


void LOOTWorker::setGame(const std::string &gameName)
{
  static std::map<std::string, GameType> gameMap = boost::assign::map_list_of
    ( "oblivion", GameType::tes4 )
    ( "fallout3", GameType::fo3 )
    ( "fallout4", GameType::fo4 )
    ( "falloutnv", GameType::fonv )
    ( "skyrim", GameType::tes5 )
	( "skyrimse", GameType::tes5se );

  auto iter = gameMap.find(ToLower(gameName));
  if (iter != gameMap.end()) {
    m_GameName = gameName;
    if(ToLower(gameName)=="skyrimse"){
      m_GameName="Skyrim Special Edition";
    }
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
void LOOTWorker::setPluginListPath(const std::string &pluginListPath){
  m_PluginListPath=pluginListPath;
}

/*void LOOTWorker::handleErr(unsigned int resultCode, const char *description)
{
  if (resultCode != LVAR(loot_ok)) {
    const char *errMessage;
    unsigned int lastError = LFUNC(loot_get_error_message)(&errMessage);
    throw std::runtime_error((boost::format("%1% failed: %2% (code %3%)") % description % errMessage % lastError).str());
  } else {
    progress(description);
  }
}*/


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

std::string LOOTWorker::repoUrl()
{
  if (m_GameId == GameType::tes4)
    return "https://github.com/loot/oblivion.git";
  else if (m_GameId == GameType::tes5)
    return "https://github.com/loot/skyrim.git";
  else if (m_GameId == GameType::fo3)
    return "https://github.com/loot/fallout3.git";
  else if (m_GameId == GameType::fonv)
    return "https://github.com/loot/falloutnv.git";
  else if(m_GameId == GameType::tes5se)
    return "https://github.com/loot/skyrimse.git";
  else if(m_GameId == GameType::fo4)
    return "https://github.com/loot/fallout4.git";
  else
    return "";
}


int LOOTWorker::run()
{

  try {
    // ensure the loot directory exists
    fs::path lootAppData = GetLOOTAppData();
    if (lootAppData.empty()) {
      errorOccured("failed to create loot app data path");
      return 1;
    }
    if (!fs::exists(lootAppData)) {
      fs::create_directory(lootAppData);
    }

    auto db=CreateDatabase(m_GameId,m_GamePath);

    bool mlUpdated = false;
    if (m_UpdateMasterlist) {
      progress("checking masterlist existence");
      if (!fs::exists(masterlistPath().parent_path())) {
        fs::create_directories(masterlistPath().parent_path());
      }
      progress("updating masterlist");
      
      mlUpdated = db->UpdateMasterlist(masterlistPath().string(),repoUrl(),"v0.10");
      
    }

    fs::path userlist = userlistPath();
    
    progress("loading lists");
    db->LoadLists(masterlistPath().string(),fs::exists(userlist)?userlistPath().string():"");
    

    progress("Evaluating lists");
    db->EvalLists();

    progress("Reading loadorder.txt");
    std::vector<std::string> pluginsList;
    std::ifstream inf(m_PluginListPath);
    if(!inf){
       errorOccured("failed to open "+ m_PluginListPath);
      return 1;
    }
    while(inf){
      std::string plugin;
      std::getline(inf,plugin);
      if(!plugin.empty()&&plugin[0]!='#')
        pluginsList.push_back(plugin);
    }
    inf.close();
    
    progress("Sorting Plugins");
    std::vector<std::string> sortedPlugins=db->SortPlugins(pluginsList);
    
    progress("Writing loadorder.txt");
    std::ofstream outf(m_PluginListPath);
    if(!outf){
       errorOccured("failed to open loadorder.txt to rewrite it");
      return 1;
    }
    outf<<"# This file was automatically generated by Mod Organizer."<<std::endl;
    for(const std::string &plugin: sortedPlugins){
      outf<<plugin<<std::endl;
    }
    outf.close();

    ptree report;

    progress("retrieving loot messages");
    for (size_t i = 0; i < sortedPlugins.size(); ++i) {
      report.add("name", sortedPlugins[i]);
      
      std::vector<SimpleMessage> pluginMessages;
      db->GetPluginMessages(sortedPlugins[i],m_Language);
      if(!pluginMessages.empty()){
        for(SimpleMessage message: pluginMessages){
          const char *type;
          if(message.type==MessageType::say){
            type="info";
          }else if(message.type==MessageType::warn){
            type= "warn";
          }else if(message.type==MessageType::error){
            type="error";
          }else{
            type = "unknown";
            errorOccured((boost::format("invalid message type %1%") % type).str());
          }
          report.add("messages.type", type);
          report.add("messages.message", message.text);
        }
      }
      
      PluginCleanliness dirtyState = db->GetPluginCleanliness(sortedPlugins[i]);
      const char *dirty;
      if(dirtyState==PluginCleanliness::clean){
        dirty="no";
      }else if(dirtyState==PluginCleanliness::dirty){
        dirty="yes";
      }else if(dirtyState==PluginCleanliness::do_not_clean){
        dirty="do not clean";
      }else
        dirty="unknown";
      report.add("dirty",dirty);
    }

    std::ofstream buf;
    buf.open(m_OutputPath.c_str());
    write_json(buf, report, false);
  } catch (const std::exception &e) {
    errorOccured((boost::format("LOOT failed: %1%") % e.what()).str());
	return 1;
  }
  progress("done");

  return 0;
}

void LOOTWorker::progress(const std::string &step)
{
  BOOST_LOG_TRIVIAL(info) << "[progress] " << step;
  fflush(stdout);
}

void LOOTWorker::errorOccured(const std::string &message)
{
  BOOST_LOG_TRIVIAL(error) << message;
  fflush(stdout);
}
