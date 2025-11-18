#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
IF(Uncrustify_PATH)
    FIND_PROGRAM(Uncrustify_EXE NAMES uncrustify
        PATHS
        ${Uncrustify_PATH}
    )
ELSE(Uncrustify_PATH)
    FIND_PROGRAM(Uncrustify_EXE NAMES uncrustify)
ENDIF(Uncrustify_PATH)

IF(Uncrustify_EXE)
    execute_process(
            COMMAND ${Uncrustify_EXE} --version
            OUTPUT_VARIABLE Uncrustify_VERSION
    )
    string(REPLACE "\n" "" Uncrustify_VERSION "${Uncrustify_VERSION}")
    string(REPLACE "uncrustify " "" Uncrustify_VERSION "${Uncrustify_VERSION}")
ENDIF(Uncrustify_EXE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Uncrustify
  VERSION_VAR Uncrustify_VERSION
  REQUIRED_VARS Uncrustify_EXE
)

if(Uncrustify_FOUND AND NOT TARGET Uncrustify::uncrustify)
  message(STATUS "uncrustify (version ${Uncrustify_VERSION}) found")
  add_executable(Uncrustify::uncrustify IMPORTED)
  set_target_properties(Uncrustify::uncrustify PROPERTIES
    IMPORTED_LOCATION "${Uncrustify_EXE}"
  )
endif()


