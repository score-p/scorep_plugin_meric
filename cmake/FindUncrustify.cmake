#
# SPDX-FileCopyrightText: (c) 2025 Forschungszentrum Jülich GmbH <fz-juelich.de>
#
# SPDX-License-Identifier: BSD-3-Clause
#
IF(UNCRUSTIFY_PATH)
    FIND_PROGRAM(UNCRUSTIFY NAMES uncrustify
        PATHS
        ${UNCRUSTIFY_PATH}
    )
ELSE(UNCRUSTIFY_PATH)
    FIND_PROGRAM(UNCRUSTIFY NAMES uncrustify)
ENDIF(UNCRUSTIFY_PATH)
IF(UNCRUSTIFY)
    message(STATUS "uncrustify found")
ENDIF(UNCRUSTIFY)
