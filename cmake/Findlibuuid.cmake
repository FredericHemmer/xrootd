#.rst:
# Findlibuuid
# -----------
#
# Find libuuid, DCE compatible Universally Unique Identifier library.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``UUID_FOUND``
#   True if libuuid has been found.
# ``UUID_INCLUDE_DIRS``
#   Where to find uuid/uuid.h.
# ``UUID_LIBRARIES``
#   The libraries to link against to use libuuid.
#
# Obsolete variables
# ^^^^^^^^^^^^^^^^^^
#
# The following variables may also be set, for backwards compatibility:
#
# ``UUID_LIBRARY``
#   where to find the libuuid library (same as UUID_LIBRARIES).
# ``UUID_INCLUDE_DIR``
#   where to find the uuid/uuid.h header (same as UUID_INCLUDE_DIRS).

include(CheckCXXSymbolExists)
include(CheckLibraryExists)
include(FindPackageHandleStandardArgs)

if(NOT UUID_INCLUDE_DIR)
  find_path(UUID_INCLUDE_DIR uuid/uuid.h)
endif()

if(EXISTS UUID_INCLUDE_DIR)
  set(UUID_INCLUDE_DIRS ${UUID_INCLUDE_DIR})
  set(CMAKE_REQUIRED_INCLUDES ${UUID_INCLUDE_DIRS})
  check_cxx_symbol_exists("uuid_generate_random" "uuid/uuid.h" _uuid_header_only)
endif()

if(NOT _uuid_header_only AND NOT UUID_LIBRARY)
  check_library_exists("uuid" "uuid_generate_random" "" _have_libuuid)
  if(_have_libuuid)
    set(UUID_LIBRARY "uuid")
    set(UUID_LIBRARIES ${UUID_LIBRARY})
  endif()
endif()

unset(CMAKE_REQUIRED_INCLUDES)
unset(_uuid_header_only)
unset(_have_libuuid)

find_package_handle_standard_args(libuuid DEFAULT_MSG UUID_INCLUDE_DIR)
mark_as_advanced(UUID_INCLUDE_DIR UUID_LIBRARY)
