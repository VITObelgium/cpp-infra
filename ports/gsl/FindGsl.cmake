# FindGsl
# --------
#
# Find the Guideline Support Library (GSL) includes.
#
# The Guideline Support Library (GSL) contains functions and types that
# are suggested for use by the C++ Core Guidelines maintained by the
# Standard C++ Foundation.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# If GSL is found, this module defines the following :prop_tgt:`IMPORTED`
# targets::
#
#  Gsl::Gsl      - The main GSL library.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project::
#
#  Gsl_FOUND          - True if GSL found on the local system
#
# Hints
# ^^^^^
#
# Set ``Gsl_ROOT_DIR`` to a directory that contains a GSL installation.
#
# This script expects to find the GSL headers at ``$Gsl_ROOT_DIR/include/gsl``.

include(FindPackageHandleStandardArgs)

find_path(Gsl_INCLUDE_DIR
    NAMES gsl/gsl
    HINTS ${Gsl_ROOT_DIR}/include ${Gsl_INCLUDEDIR}
)

find_package_handle_standard_args(Gsl
    FOUND_VAR Gsl_FOUND
    REQUIRED_VARS Gsl_INCLUDE_DIR
)

mark_as_advanced(Gsl_ROOT_DIR Gsl_INCLUDE_DIR)

if(Gsl_FOUND AND NOT TARGET Gsl::Gsl)
    add_library(Gsl::Gsl INTERFACE IMPORTED)
    set_target_properties(Gsl::Gsl PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Gsl_INCLUDE_DIR}"
    )
endif()
