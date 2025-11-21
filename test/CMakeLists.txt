#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# Instrument an example program with Score-P, and collect a measurement with the plugin
#
# This could be improved by writing it in a more CMake-like way.
# Right now, we almost hardcode the compile command, using the `scorep-config` command to
# get the scorep executable and mpicc wrapper.
find_package(Scorep REQUIRED)
execute_process(COMMAND ${SCOREP_CONFIG} "--mpicc" OUTPUT_VARIABLE SCOREP_CONFIG_MPICC)
execute_process(COMMAND ${SCOREP_CONFIG} "--prefix" OUTPUT_VARIABLE SCOREP_CONFIG_PREFIX)

add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/adv.x
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/adv.c
  COMMAND ${SCOREP_CONFIG_PREFIX}/bin/scorep
          ${SCOREP_CONFIG_MPICC} -DUSE_MPI -DUSE_OMP -g -O2 -fopenmp -lm -o adv.x ${CMAKE_CURRENT_SOURCE_DIR}/adv.c
)

add_custom_target(run_plugin
  DEPENDS ${PROJECT_BINARY_DIR}/adv.x meric_plugin
  COMMAND LD_LIBRARY_PATH="$<TARGET_FILE_DIR:meric_plugin>:$LD_LIBRARY_PATH"
          SCOREP_METRIC_PLUGINS=meric_plugin
          SCOREP_METRIC_MERIC_PLUGIN=RAPL:package_0
          SCOREP_METRIC_MERIC_PLUGIN_INTERVAL_US=20000
          SCOREP_METRIC_MERIC_PLUGIN_DOMAINS=RAPL,
          SCOREP_ENABLE_PROFILING=0
          SCOREP_ENABLE_TRACING=1
          SCOREP_EXPERIMENT_DIRECTORY=scorep-measurement
          mpirun -n 2 ./adv.x
)

add_executable(meric_example_basic
  meric_example_basic.cpp
)
target_link_libraries(meric_example_basic
  PUBLIC
  Meric::libmeric_ext
)
