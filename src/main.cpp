#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
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


int main(int argc, char *argv[])
{
  logging::core::get()->set_filter(
    logging::trivial::severity >= logging::trivial::info
  );

  std::vector<std::string> arguments;
  std::copy(argv + 1, argv + argc, std::back_inserter(arguments));

  LOOTWorker worker;

  try {
    worker.setGame(getParameter<std::string>(arguments, "game"));
    worker.setGamePath(getParameter<std::string>(arguments, "gamePath"));
//    worker.setMasterlist(getParameter<std::string>(arguments, "masterlist"));
//    worker.setUserlist(getParameter<std::string>(arguments, "userlist"));

    return worker.run();
  } catch (const std::exception &e) {
    BOOST_LOG_TRIVIAL(error) << "Error: " << e.what();
    return 1;
  }
}
