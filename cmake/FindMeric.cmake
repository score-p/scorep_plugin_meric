#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#[=======================================================================[.rst:
FindMeric
-------

Finds MERIC energy libraries

Input Variables
^^^^^^^^^^^^^^^

``Meric_ROOT``
  Meric installation path.


Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Meric::libmeric``
  The Meric library

Result Variables
^^^^^^^^^^^^^^^^

This module defines the following variables:

``Meric_FOUND``
  Boolean indicating whether Meric was found.
``Meric_INCLUDE_DIRS``
  Include directories needed to use Meric
``Meric_LIBRARIES``
  Libraries needed to link to Meric

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``Meric_INCLUDE_DIR``
  The directory containing ``meric.h``.
``Meric_LIBRARY``
  The full path to libmeric.
``Meric_EXT_DIR``
  The directory containing ``meric_ext.h``. Usually identical to ``Meric_INCLUDE_DIR``
``Meric_EXT_LIB``
  The full path to libmeric_ext.


#]=======================================================================]
find_path(Meric_INCLUDE_DIR
    NAMES meric.h
)
find_path(Meric_EXT_DIR
    NAMES meric_ext.h
)
find_library(Meric_LIBRARY
    NAMES meric
)
find_library(Meric_EXT_LIB
    NAMES meric_ext
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Meric
    REQUIRED_VARS
        Meric_INCLUDE_DIR
        Meric_EXT_DIR
        Meric_LIBRARY
        Meric_EXT_LIB
)

if(Meric_FOUND)
    set(Meric_LIBRARIES ${Meric_LIBRARY} ${Meric_EXT_LIB})
    if(${Meric_INCLUDE_DIR} STREQUAL ${Meric_EXT_DIR})
        set(Meric_INCLUDE_DIRS ${Meric_INCLUDE_DIR})
    else()
        set(Meric_INCLUDE_DIRS ${Meric_INCLUDE_DIR} ${Meric_EXT_DIR})
    endif()
    message(STATUS "meric include path: ${Meric_INCLUDE_DIRS}")
    message(STATUS "meric libs path: ${Meric_LIBRARIES}")
endif()

if(Meric_FOUND AND NOT TARGET Meric::libmeric)
    add_library(Meric::libmeric UNKNOWN IMPORTED)
    set_target_properties(Meric::libmeric PROPERTIES
        IMPORTED_LOCATION "${Meric_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Meric_INCLUDE_DIR}"
        INTERFACE_LINK_OPTIONS "-fopenmp"
    )
    add_library(Meric::libmeric_ext UNKNOWN IMPORTED)
    set_target_properties(Meric::libmeric_ext PROPERTIES
        IMPORTED_LOCATION "${Meric_EXT_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${Meric_EXT_DIR}"
        INTERFACE_LINK_OPTIONS "-fopenmp"
    )
endif()

mark_as_advanced(
  Meric_INCLUDE_DIR
  Meric_LIBRARY
  Meric_EXT_DIR
  Meric_EXT_LIB
)
