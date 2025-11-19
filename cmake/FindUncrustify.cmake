#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#[=======================================================================[.rst:
FindUncrustify
--------------

Finds the uncrustify program (a source code formatter)

Input Variables
^^^^^^^^^^^^^^^

``Uncrustify_ROOT``
  Installation path.


Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Uncrustify::uncrustify``
  The ``uncrustify`` program

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Uncrustify_FOUND``
  Boolean indicating whether Uncrustify was found.
``Uncrustify_VERSION``
  Version reported by ``uncrustify --version``.
``Uncrustify_EXECUTABLE``
  The path of the ``uncrustify`` program.

#]=======================================================================]

find_program(Uncrustify_EXECUTABLE NAMES uncrustify)

if(Uncrustify_EXECUTABLE)
  execute_process(
    COMMAND ${Uncrustify_EXECUTABLE} --version
    OUTPUT_VARIABLE Uncrustify_VERSION)
  string(REPLACE "\n" "" Uncrustify_VERSION "${Uncrustify_VERSION}")
  string(REPLACE "uncrustify " "" Uncrustify_VERSION "${Uncrustify_VERSION}")
endif(Uncrustify_EXECUTABLE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Uncrustify
  VERSION_VAR Uncrustify_VERSION
  REQUIRED_VARS Uncrustify_EXECUTABLE)

if(Uncrustify_FOUND AND NOT TARGET Uncrustify::uncrustify)
  add_executable(Uncrustify::uncrustify IMPORTED)
  set_target_properties(Uncrustify::uncrustify PROPERTIES
    IMPORTED_LOCATION "${Uncrustify_EXECUTABLE}")
endif()

mark_as_advanced(
  Uncrustify_EXECUTABLE
  Uncrustify_VERSION)


