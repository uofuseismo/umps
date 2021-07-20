# Already in cache, be silent
if (TIME_INCLUDE_DIR AND TIME_LIBRARY)
    set(TIME_FIND_QUIETLY TRUE)
endif()

# Find the include directory
find_path(TIME_INCLUDE_DIR
          NAMES time
          HINTS $ENV{TIME_ROOT}/include
                $ENV{TIME_ROOT}/
                /usr/local/include)
# Find the library components
find_library(TIME_LIBRARY
             NAMES time
             PATHS $ENV{TIME}/lib/
                   $ENV{TIME}/
                   /usr/local/lib64
                   /usr/local/lib
            )
# Handle the QUIETLY and REQUIRED arguments and set MKL_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FindTime
                                  DEFAULT_MSG TIME_INCLUDE_DIR TIME_LIBRARY)
mark_as_advanced(TIME_INCLUDE_DIR TIME_LIBRARY)
