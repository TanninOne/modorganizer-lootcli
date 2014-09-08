#include "lootthread.h"
#include "../uibase/scopeguard.h"
#pragma warning (push, 0)
#include <api/api.h>
#include <backend/graph.h>
#include <backend/streams.h>
#include <backend/metadata.h>
#include <backend/parsers.h>
#include <backend/generators.h>
#include <backend/globals.h>
#pragma warning (pop)
#include <map>
#include <list>
#include <boost/assign.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace std;
using namespace loot;
namespace fs = boost::filesystem;


struct _loot_db_int : public loot::Game {};


LOOTWorker::LOOTWorker()
  : m_GameId(loot_game_tes5)
  , m_Language(loot_lang_any)
{
}



std::string ToLower(const std::string &text)
{
  std::string result = text;
  std::transform(text.begin(), text.end(), result.begin(), tolower);
  return result;
}


const char *lootErrorString(unsigned int errorCode)
{
  // can't use switch-case here because the loot_error constants are externs
  if (errorCode == loot_error_liblo_error)         return "There was an error in performing a load order operation.";
  if (errorCode == loot_error_file_write_fail)     return "A file could not be written to.";
  if (errorCode == loot_error_parse_fail)          return "There was an error parsing the file.";
  if (errorCode == loot_error_condition_eval_fail) return "There was an error evaluating the conditionals in a metadata file.";
  if (errorCode == loot_error_regex_eval_fail)     return "There was an error evaluating the regular expressions in a metadata file.";
  if (errorCode == loot_error_no_mem)              return "The API was unable to allocate the required memory.";
  if (errorCode == loot_error_invalid_args)        return "Invalid arguments were given for the function.";
  if (errorCode == loot_error_no_tag_map)          return "No Bash Tag map has been generated yet.";
  if (errorCode == loot_error_path_not_found)      return "A file or folder path could not be found.";
  if (errorCode == loot_error_no_game_detected)    return "The given game could not be found.";
  if (errorCode == loot_error_windows_error)       return "An error occurred during a call to the Windows API.";
  if (errorCode == loot_error_sorting_error)       return "An error occurred while sorting plugins.";
  return "Unknown error code";
}


void LOOTWorker::setGame(const std::string &gameName)
{
  static std::map<std::string, unsigned int> gameMap = boost::assign::map_list_of
    ( "oblivion", loot_game_tes4 )
    ( "fallout3", loot_game_fo3 )
    ( "falloutnv", loot_game_fonv )
    ( "skyrim", loot_game_tes5 );

  auto iter = gameMap.find(ToLower(gameName));
  if (iter != gameMap.end()) {
    m_GameId = iter->second;
  } else {
    throw std::runtime_error((boost::format("invalid game name \"%1%\"") % gameName).str());
  }
}

void LOOTWorker::setGamePath(const std::string &gamePath)
{
  m_GamePath = gamePath;
}

void LOOTWorker::setMasterlist(const std::string &masterlistPath)
{
  m_MasterlistPath = masterlistPath;
}

void LOOTWorker::setUserlist(const std::string &userlistPath)
{
  m_UserlistPath = userlistPath;
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
  if (resultCode != loot_ok) {
    const char *errMessage;
    unsigned int lastError = loot_get_error_message(&errMessage);
    throw std::runtime_error((boost::format("%1% failed: %2% (code %3%)") % description % errMessage % lastError).str());
  } else {
    progress(description);
  }
}


//
// The following code is an almost exact copy of code from the loot ui
// copyright by WrinklyNinja
//

/*
struct plugin_loader {
    plugin_loader(loot::Plugin& plugin, loot::Game& game) : _plugin(plugin), _game(game) {
    }

    void operator () () {
        _plugin = loot::Plugin(_game, _plugin.Name(), false);
    }

    loot::Plugin& _plugin;
    loot::Game& _game;
    string _filename;
    bool _b;
};

struct plugin_list_loader {
    plugin_list_loader(PluginGraph& graph, loot::Game& game) : _graph(graph), _game(game) {}

    void operator () () {
        loot::vertex_it vit, vitend;
        for (boost::tie(vit, vitend) = boost::vertices(_graph); vit != vitend; ++vit) {
            std::string name = _graph[*vit].Name();
            if (skipPlugins.find(name) == skipPlugins.end()) {
              // TODO this seems to crash if loot doesn't like the plugin.
              _graph[*vit] = loot::Plugin(_game, name, false);
            }
        }
    }

    PluginGraph& _graph;
    loot::Game& _game;
    set<string> skipPlugins;
};


struct masterlist_updater_parser {
  masterlist_updater_parser(bool doUpdate, loot::Game& game, list<loot::Message>& errors, list<loot::Plugin>& plugins, list<loot::Message>& messages, string& revision) : _doUpdate(doUpdate), _game(game), _errors(errors), _plugins(plugins), _messages(messages), _revision(revision) {}

  void operator () () {

    if (_doUpdate) {
      BOOST_LOG_TRIVIAL(debug) << "Updating masterlist";
      try {
        _revision = UpdateMasterlist(_game, _errors, _plugins, _messages);
        BOOST_LOG_TRIVIAL(debug) << "got revision: " << _revision;
      } catch (loot::error& e) {
        _plugins.clear();
        _messages.clear();
        BOOST_LOG_TRIVIAL(error) << "Masterlist update failed. Details: " << e.what();
        _errors.push_back(loot::Message(loot::Message::error, (boost::format("Masterlist update failed. Details: %1%") % e.what()).str()));
        //Try getting masterlist revision anyway.
        try {
          _revision = GetMasterlistRevision(_game);
        }
        catch (loot::error& e) {
          BOOST_LOG_TRIVIAL(error) << "Masterlist revision check failed. Details: " << e.what();
          _errors.push_back(loot::Message(loot::Message::error, (boost::format("Masterlist revision check failed. Details: %1%") %e.what()).str()));
        }
      }
    }
    else {
      BOOST_LOG_TRIVIAL(debug) << "Getting masterlist revision";
      try {
        _revision = GetMasterlistRevision(_game);
      }
      catch (loot::error& e) {
        BOOST_LOG_TRIVIAL(error) << "Masterlist revision check failed. Details: " << e.what();
        _errors.push_back(loot::Message(loot::Message::error, (boost::format("Masterlist revision check failed. Details: %1%") % e.what()).str()));
      }
    }

    if (_plugins.empty() && _messages.empty() && fs::exists(_game.MasterlistPath())) {
      BOOST_LOG_TRIVIAL(debug) << "Parsing masterlist...";
      try {
        loot::ifstream in(_game.MasterlistPath());
        YAML::Node mlist = YAML::Load(in);
        in.close();

        if (mlist["globals"])
          _messages = mlist["globals"].as< list<loot::Message> >();
        if (mlist["plugins"])
          _plugins = mlist["plugins"].as< list<loot::Plugin> >();
      } catch (YAML::Exception& e) {
        BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Details: " << e.what();
        _errors.push_back(loot::Message(loot::Message::error, (boost::format("Masterlist parsing failed. Details: %1%") % e.what()).str()));
      }
      BOOST_LOG_TRIVIAL(debug) << "Finished parsing masterlist.";

    }

    if (_revision.empty()) {
      if (fs::exists(_game.MasterlistPath()))
        _revision = "Unknown";
      else
        _revision = "No masterlist";
    }
  }

  bool _doUpdate;
  loot::Game& _game;
  list<loot::Message>& _errors;
  list<loot::Plugin>& _plugins;
  list<loot::Message>& _messages;
  string& _revision;
};


bool LOOTWorker::sort(loot::Game &game)
{
  YAML::Node ulist;
  list<loot::Message> messages, mlist_messages, ulist_messages;
  list<loot::Plugin> mlist_plugins, ulist_plugins;
  list<loot::Plugin> plugins;
  boost::thread_group group;
  loot::PluginGraph graph;
  string revision;

  progress("preparing sort");
  ///////////////////////////////////////////////////////
  // Load Plugins & Lists
  ///////////////////////////////////////////////////////

  masterlist_updater_parser mup(m_UpdateMasterlist, game, messages, mlist_plugins, mlist_messages, revision);
  boost::thread *mupThread = group.create_thread(mup);

  //First calculate the mean plugin size. Store it temporarily in a map to reduce filesystem lookups and file size recalculation.
  size_t meanFileSize = 0;
  boost::unordered_map<std::string, size_t> tempMap;
  for (fs::directory_iterator it(game.DataPath()); it != fs::directory_iterator(); ++it) {
    if (fs::is_regular_file(it->status()) && IsPlugin(it->path().string())) {

      size_t fileSize = static_cast<size_t>(fs::file_size(it->path()));
      meanFileSize += fileSize;

      tempMap.emplace(it->path().filename().string(), fileSize);
      plugins.push_back(loot::Plugin(it->path().filename().string()));  //Just in case there's an error with the graph.
    }
  }
  meanFileSize /= tempMap.size();

  //Now load plugins.
  plugin_list_loader pll(graph, game);
  for (boost::unordered_map<string, size_t>::const_iterator it = tempMap.begin(), endit = tempMap.end(); it != endit; ++it) {
    BOOST_LOG_TRIVIAL(debug) << "Found plugin: " << it->first.c_str();
    progress((boost::format("loading %1%") % it->first).str());

    vertex_t v = boost::add_vertex(loot::Plugin(it->first), graph);

    if (it->second > meanFileSize) {
      pll.skipPlugins.insert(it->first);
      plugin_loader pl(graph[v], game);
      group.create_thread(pl);
    }
  }
  progress("waiting for masterlist update");
  group.create_thread(pll);
  while (!mupThread->try_join_for(boost::chrono::milliseconds(100))) {
    progress("waiting for masterlist update");
  }
  group.join_all();

  progress("load userlist");

  //Now load userlist.
  if (fs::exists(game.UserlistPath())) {
    BOOST_LOG_TRIVIAL(debug) << "Parsing userlist at: " << game.UserlistPath().c_str();

    try {
      loot::ifstream in(game.UserlistPath());
      YAML::Node ulist = YAML::Load(in);
      in.close();

      if (ulist["plugins"])
        ulist_plugins = ulist["plugins"].as< list<loot::Plugin> >();
    } catch (YAML::ParserException& e) {
      BOOST_LOG_TRIVIAL(error) << "Userlist parsing failed. Details: " << e.what();
      messages.push_back(loot::Message(loot::Message::error, (boost::format("Userlist parsing failed. Details: %1%") % e.what()).str()));
    }
    if (ulist["plugins"])
      ulist_plugins = ulist["plugins"].as< list<loot::Plugin> >();
  }

  progress("merge & check metadata");

  ///////////////////////////////////////////////////////
  // Merge & Check Metadata
  ///////////////////////////////////////////////////////

  if (fs::exists(game.MasterlistPath()) || fs::exists(game.UserlistPath())) {
    //Merge all global message lists.
    BOOST_LOG_TRIVIAL(debug) << "Merging all global message lists.";
    if (!mlist_messages.empty())
      messages.insert(messages.end(), mlist_messages.begin(), mlist_messages.end());
    if (!ulist_messages.empty())
      messages.insert(messages.end(), ulist_messages.begin(), ulist_messages.end());

    //Evaluate any conditions in the global messages.
    BOOST_LOG_TRIVIAL(debug) << "Evaluating global message conditions.";
    try {
      list<loot::Message>::iterator it = messages.begin();
      while (it != messages.end()) {
        if (!it->EvalCondition(game, m_Language))
          it = messages.erase(it);
        else
          ++it;
      }
    } catch (loot::error& e) {
      BOOST_LOG_TRIVIAL(error) << "A global message contains a condition that could not be evaluated. Details: " << e.what();
      messages.push_back(loot::Message(loot::Message::error, (boost::format("A global message contains a condition that could not be evaluated. Details: %1%") %e.what()).str()));
    }

    //Merge plugin list, masterlist and userlist plugin data.
    BOOST_LOG_TRIVIAL(debug) << "Merging plugin list, masterlist and userlist data, evaluating conditions and checking for install validity.";
    loot::vertex_it vit, vitend;
    for (boost::tie(vit, vitend) = boost::vertices(graph); vit != vitend; ++vit) {
      BOOST_LOG_TRIVIAL(debug) << "Merging for plugin \"" << graph[*vit].Name().c_str() << "\"";

      //Check if there is a plugin entry in the masterlist. This will also find matching regex entries.
      list<loot::Plugin>::iterator pos = std::find(mlist_plugins.begin(), mlist_plugins.end(), graph[*vit]);

      if (pos != mlist_plugins.end()) {
        BOOST_LOG_TRIVIAL(debug) << "Merging masterlist data down to plugin list data.";
        graph[*vit].MergeMetadata(*pos);
      }

      //Check if there is a plugin entry in the userlist. This will also find matching regex entries.
      pos = std::find(ulist_plugins.begin(), ulist_plugins.end(), graph[*vit]);

      if (pos != ulist_plugins.end() && pos->Enabled()) {
        BOOST_LOG_TRIVIAL(debug) << "Merging userlist data down to plugin list data.";
        graph[*vit].MergeMetadata(*pos);
      }

      progress("Evaluate conditions for merged plugin data");

      //Now that items are merged, evaluate any conditions they have.
      BOOST_LOG_TRIVIAL(debug) << "Evaluate conditions for merged plugin data.";
      try {
        graph[*vit].EvalAllConditions(game, m_Language);
      } catch (loot::error &e) {
        BOOST_LOG_TRIVIAL(error) << "\"" << graph[*vit].Name().c_str() << "\" contains a condition that could not be evaluated. Details: " << e.what();
        messages.push_back(loot::Message(loot::Message::error, (boost::format("\"%1%\" contains a condition that could not be evaluated. Details: %2%") % graph[*vit].Name() % e.what()).str()));
      }

      progress("Check install validity");

      //Also check install validity.
      BOOST_LOG_TRIVIAL(debug) << "Checking that the current install is valid according to this plugin's data.";
      graph[*vit].CheckInstallValidity(game);

    }
  }

  progress("Building plugin graph...");

  ///////////////////////////////////////////////////////
  // Build Graph Edges & Sort
  ///////////////////////////////////////////////////////

  //Check for back-edges, then perform a topological sort.
  try {
    BOOST_LOG_TRIVIAL(debug) << "Building the plugin dependency graph...";

    //Now add the interactions between plugins to the graph as edges.
    BOOST_LOG_TRIVIAL(debug) << "Adding non-overlap edges.";
    AddSpecificEdges(graph);

    BOOST_LOG_TRIVIAL(debug) << "Adding priority edges.";
    AddPriorityEdges(graph);

    BOOST_LOG_TRIVIAL(debug) << "Adding overlap edges.";
    AddOverlapEdges(graph);

    BOOST_LOG_TRIVIAL(debug) << "Checking to see if the graph is cyclic.";
    loot::CheckForCycles(graph);

    progress("Performing sort...");

    BOOST_LOG_TRIVIAL(debug) << "Performing a topological sort.";
    loot::Sort(graph, plugins);

    BOOST_LOG_TRIVIAL(debug) << "Setting load order.";
    try {
      game.SetLoadOrder(plugins);
    }
    catch (loot::error& e) {
      BOOST_LOG_TRIVIAL(error) << "Failed to set the load order. Details: " << e.what();
      messages.push_back(loot::Message(loot::Message::error, (boost::format("Failed to set the load order. Details: %1%") % e.what()).str()));
    }
    BOOST_LOG_TRIVIAL(debug) << "Load order set:";
    for (list<loot::Plugin>::iterator it = plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
      BOOST_LOG_TRIVIAL(debug) << '\t' << it->Name().c_str();
    }
    progress("Generating Report");
    GenerateReport(game.ReportPath(), messages, plugins, revision, m_UpdateMasterlist);
    return true;
  } catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << "Failed to calculate the load order. Details: " << e.what();
    messages.push_back(loot::Message(loot::Message::error, (boost::format("Failed to calculate the load order. Details: %1%") % e.what()).str()));
    return false;
  }
}
*/
//
// end of copied code
//

void LOOTWorker::run()
{
  try {
    // ensure the loot directory exists
    boost::filesystem::create_directory(g_path_local);

/*
    loot::Game game(m_GameId);
    game.SetPath(m_GamePath);
    game.RefreshActivePluginsList();*/

    loot_db db;
    loot_create_db(&db, m_GameId, m_GamePath.c_str());
    ON_BLOCK_EXIT([&] () {
      loot_destroy_db(db);
      loot_cleanup();
    });

    bool mlUpdated = false;
    if (m_UpdateMasterlist) {
      char *masterlistPath = nullptr;
      char *repoURL = nullptr;
      char *repoBranch = nullptr;
      ON_BLOCK_EXIT([&] () {
        loot_free_string(masterlistPath);
        loot_free_string(repoURL);
        loot_free_string(repoBranch);
      });
      if ((loot_get_masterlist_path(db, &masterlistPath) == loot_ok)
          && (loot_get_repository_url(db, &repoURL) == loot_ok)
          && (loot_get_repository_branch(db, &repoBranch) == loot_ok)) {
        unsigned int res = loot_update_masterlist(db
                                                  , masterlistPath
                                                  , repoURL
                                                  , repoBranch
                                                  , &mlUpdated);
        if (res != loot_ok) {
          progress((boost::format("failed to update masterlist: %1%") % lootErrorString(res)).str());
        }
      } else {
        const char *errorMessage = nullptr;
        loot_get_error_message(&errorMessage);
        errorOccured((boost::format("failed to query paths: %1%") % errorMessage).str());
      }
    }

    char **sortedPlugins;
    size_t numPlugins;

    loot_sort_plugins(db, &sortedPlugins, &numPlugins);

/*    if (sort(game)) {
      BOOST_LOG_TRIVIAL(info) << "[report] " << game.ReportPath().string();
    }*/
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
