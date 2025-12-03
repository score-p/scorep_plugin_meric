#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
cmake_minimum_required(VERSION 3.12)
project(meric_plugin
  VERSION 0.0.0
  DESCRIPTION "Score-P metric plugin to read energy values with MERIC ExtLib"
  HOMEPAGE_URL "https://gitlab.jsc.fz-juelich.de/corbin1/scorep_plugin_meric"
  LANGUAGES CXX C)

include(cmake/PreventInSourceBuilds.cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scorep_plugin_common")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scorep_plugin_cxx_wrapper")

if ( MERIC_PLUGIN_DEVELOPER_MODE )
  find_package(Uncrustify)
  find_package(Reuse)
endif()

find_package(Meric)

# Intialize git submodules if not done already
include(cmake/GitSubmoduleUpdate.cmake)
git_submodule_update()

add_subdirectory(scorep_plugin_cxx_wrapper)

set(CMAKE_CXX_EXTENSIONS OFF)

set(MERIC_PLUGIN_SRC
    src/energy_metric.cpp
    src/energy_metric.h
    src/extlib_wrapper.cpp
    src/extlib_wrapper.h
    src/meric_measurement.cpp
    src/meric_measurement.h
    src/meric_plugin.cpp
    src/meric_plugin.h
    src/utils.cpp
    src/utils.h
    )


add_library(meric_plugin SHARED ${MERIC_PLUGIN_SRC})
target_compile_features(meric_plugin PUBLIC cxx_std_14)
target_compile_options(meric_plugin INTERFACE -Wall -pedantic -Wextra)
target_link_libraries(meric_plugin PUBLIC
  scorep-plugin-cxx
  Meric::libmeric_ext)
target_include_directories(meric_plugin PUBLIC include)

add_executable(show_counters src/show_counters.cpp ${MERIC_PLUGIN_SRC})
target_compile_features(show_counters PUBLIC cxx_std_14)
target_compile_options(show_counters INTERFACE -Wall -pedantic -Wextra)
target_link_libraries(show_counters PUBLIC
  scorep-plugin-cxx
  Meric::libmeric_ext)

include_directories(include)

install(TARGETS meric_plugin
    LIBRARY DESTINATION lib)

install(TARGETS show_counters
    RUNTIME DESTINATION bin )

if ( MERIC_PLUGIN_DEVELOPER_MODE )
  if(Uncrustify_FOUND)
    add_custom_command(
      TARGET meric_plugin
      PRE_BUILD
      COMMAND Uncrustify::uncrustify
      ARGS -c ${CMAKE_CURRENT_SOURCE_DIR}/uncrustify.cfg --replace --no-backup ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
  endif(Uncrustify_FOUND)
  if(Reuse_FOUND)
    add_custom_command(
      TARGET meric_plugin
      POST_BUILD
      COMMAND Reuse::reuse
      ARGS lint)
  endif(Reuse_FOUND)
endif( MERIC_PLUGIN_DEVELOPER_MODE )


add_subdirectory(test)
