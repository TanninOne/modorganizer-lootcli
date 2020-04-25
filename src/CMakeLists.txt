cmake_minimum_required(VERSION 3.16)
include(../../cmake_common/src.cmake)

set(header_files ${header_files} ../include/lootcli/lootcli.h)
requires_library(loot cpptoml)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
