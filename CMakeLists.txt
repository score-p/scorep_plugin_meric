#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
cmake_minimum_required(VERSION 3.12)
project(meric_plugin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scorep_plugin_common")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scorep_plugin_cxx_wrapper")

find_package(Uncrustify)
find_package(Meric)

# Intialize git submodules if not done already
include(cmake/GitSubmoduleUpdate.cmake)
git_submodule_update()

add_subdirectory(scorep_plugin_cxx_wrapper)

set(CMAKE_CXX_EXTENSIONS OFF)

set(MERIC_PLUGIN_SRC
    src/meric_plugin.cpp
    src/meric_plugin.h
    src/meric_measurement.cpp
    src/meric_measurement.h
    src/energy_metric.cpp
    src/energy_metric.h
    src/utils.cpp
    src/utils.h
    )


add_library(meric_plugin SHARED ${MERIC_PLUGIN_SRC})
target_compile_features(meric_plugin PUBLIC cxx_std_14)
target_link_libraries(meric_plugin PUBLIC
  scorep-plugin-cxx
  Meric::libmeric_ext)
target_include_directories(meric_plugin PUBLIC include)

include_directories(include)

install(TARGETS meric_plugin
    LIBRARY DESTINATION lib)

IF(Uncrustify_FOUND)
add_custom_command(
  TARGET meric_plugin
  PRE_BUILD
  COMMAND Uncrustify::uncrustify
  ARGS -c ${CMAKE_CURRENT_SOURCE_DIR}/uncrustify.cfg --replace --no-backup ${CMAKE_CURRENT_SOURCE_DIR}/src/*
  )
ENDIF(Uncrustify_FOUND)


add_subdirectory(test)
