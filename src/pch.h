#pragma warning(disable: 4251)  // neds to have dll-interface
#pragma warning(disable: 4355)  // this used in initializer list
#pragma warning(disable: 4371)  // layout may have changed
#pragma warning(disable: 4514)  // unreferenced inline function removed
#pragma warning(disable: 4571)  // catch semantics changed
#pragma warning(disable: 4619)  // no warning X
#pragma warning(disable: 4623)  // default constructor deleted
#pragma warning(disable: 4625)  // copy constructor deleted
#pragma warning(disable: 4626)  // copy assignment operator deleted
#pragma warning(disable: 4710)  // function not inlined
#pragma warning(disable: 4820)  // padding
#pragma warning(disable: 4866)  // left-to-right evaluation order
#pragma warning(disable: 4868)  // left-to-right evaluation order
#pragma warning(disable: 5026)  // move constructor deleted
#pragma warning(disable: 5027)  // move assignment operator deleted
#pragma warning(disable: 5045)  // spectre mitigation

#pragma warning(push, 3)
#pragma warning(disable: 4365)  // signed/unsigned mismatch
#pragma warning(disable: 4774)  // bad format string
#pragma warning(disable: 4946)  // reinterpret_cast used between related classes
#pragma warning(disable: 4800)  // implicit conversion

// std
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// qt
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>

// boost
#include <boost/algorithm/string.hpp>
//#include <boost/assign.hpp>
//#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
//#include <boost/log/core.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/log/utility/setup/console.hpp>
//#include <boost/log/utility/setup/file.hpp>

// loot
#include <loot/api.h>
#include <loot/enum/game_type.h>

// third-party
#include <toml++/toml.h>

// windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>
#include <fcntl.h>
#include <io.h>
#include <shellapi.h>

#undef _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#pragma warning(pop)
