#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#[=======================================================================[.rst:
FindReuse
--------------

Finds the reuse program (a linter that checks project compliance with the reuse specification)

Input Variables
^^^^^^^^^^^^^^^

``Reuse_ROOT``
  Installation path.


Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Reuse::reuse``
  The ``reuse`` program

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Reuse_FOUND``
  Boolean indicating whether Reuse was found.
``Reuse_VERSION``
  Version reported by ``reuse --version``.
``Reuse_EXECUTABLE``
  The path of the ``reuse`` program.

#]=======================================================================]

find_program(Reuse_EXECUTABLE NAMES reuse)
if(Reuse_EXECUTABLE)
  execute_process(
    COMMAND ${Reuse_EXECUTABLE} --version
    OUTPUT_VARIABLE Reuse_VERSION)
  # Program output contains the version in line one and is followed by several
  # more lines, which we have to cut here.
  string(REGEX REPLACE "reuse, +version +([^\n]+).*" "\\1" Reuse_VERSION "${Reuse_VERSION}")
endif(Reuse_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Reuse
  VERSION_VAR Reuse_VERSION
  REQUIRED_VARS Reuse_EXECUTABLE)

if(Reuse_FOUND AND NOT TARGET Reuse::reuse)
  add_executable(Reuse::reuse IMPORTED)
  set_target_properties(Reuse::reuse PROPERTIES
    IMPORTED_LOCATION "${Reuse_EXECUTABLE}")
endif()

mark_as_advanced(
  Reuse_EXECUTABLE
  Reuse_VERSION)
