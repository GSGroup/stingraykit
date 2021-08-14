# Tries to find libunwind.
# Usage:
#     find_package(libunwind)
# Input params:
#  libunwind_ROOT_DIR           libunwind location hint (optional)
# Output:
#  libunwind_FOUND              System has libunwind libs/headers
#  libunwind_LIBRARIES          The libunwind libraries
#  libunwind_INCLUDE_DIR        The location of libunwind headers

find_library(libunwind_core
  NAMES unwind
  HINTS ${libunwind_ROOT_DIR}/lib)

find_library(libunwind_arch
  NAMES unwind-mips unwind-arm
  HINTS ${libunwind_ROOT_DIR}/lib)

find_path(libunwind_INCLUDE_DIR
  NAMES libunwind.h
  HINTS ${libunwind_ROOT_DIR}/include)

set(libunwind_LIBRARIES ${libunwind_core} ${libunwind_arch})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  libunwind
  DEFAULT_MSG
  libunwind_LIBRARIES
  libunwind_INCLUDE_DIR)

mark_as_advanced(
  libunwind_ROOT_DIR
  libunwind_LIBRARIES
  libunwind_INCLUDE_DIR)
