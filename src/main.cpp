#include <exception>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/locale.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#include "lootthread.h"

using namespace std;
namespace logging = boost::log;


template <typename T>
T getParameter(const std::vector<std::string> &arguments, const std::string &key)
{
  auto iter = std::find(arguments.begin(), arguments.end(), std::string("--") + key);
  if ((iter != arguments.end())
      && ((iter + 1) != arguments.end())) {
    return boost::lexical_cast<T>(*(iter + 1));
  } else {
    throw std::runtime_error(std::string("argument missing " + key));
  }
}

template <>
bool getParameter<bool>(const std::vector<std::string> &arguments, const std::string &key)
{
  auto iter = std::find(arguments.begin(), arguments.end(), std::string("--") + key);
  if (iter != arguments.end()) {
    return true;
  } else {
    return false;
  }
}


int main(int argc, char *argv[])
{
  boost::log::add_console_log(std::cout, boost::log::keywords::format = "%Message%");
  logging::core::get()->set_filter(
    logging::trivial::severity >= logging::trivial::info
  );

  std::vector<std::string> arguments;
  std::copy(argv + 1, argv + argc, std::back_inserter(arguments));

  // design rationale: this was designed to have the actual loot stuff run in a separate thread. That turned
  // out to be unnecessary atm.

  try {
    LOOTWorker worker;
    worker.setUpdateMasterlist(!getParameter<bool>(arguments, "skipUpdateMasterlist"));
    worker.setGame(getParameter<std::string>(arguments, "game"));
    worker.setGamePath(getParameter<std::string>(arguments, "gamePath"));
    worker.setPluginListPath(getParameter<std::string>(arguments, "pluginListPath"));
    worker.setOutput(getParameter<std::string>(arguments, "out"));
    return worker.run();
  } catch (const std::exception &e) {
    BOOST_LOG_TRIVIAL(error) << "Error: " << e.what();
    return 1;
  }
}
