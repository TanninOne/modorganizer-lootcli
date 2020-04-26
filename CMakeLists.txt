cmake_minimum_required(VERSION 3.16)

project(lootcli)
set(project_type exe)
set(install_dir bin/loot)

include(../cmake_common/project.cmake)
add_subdirectory(src)
