# Already in cache, be silent
if (MINISEED_INCLUDE_DIR AND MINISEED_LIBRARY)
    set(MINISEED_FIND_QUIETLY TRUE)
endif()

set(MINISEED mseed)

# Find the include directory
find_path(MINISEED_INCLUDE_DIR
          NAMES libmseed.h
          HINTS $ENV{MINISEED_ROOT}/include
                $ENV{MINISEED_ROOT}/
                /usr/local/include)
# Find the library components
find_library(MINISEED_LIBRARY
             NAMES ${MINISEED}
             PATHS $ENV{MINISEED}/lib/
                   $ENV{MINISEED}/
                   /usr/local/lib64
                   /usr/local/lib
            )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FindMiniSEED
                                  DEFAULT_MSG MINISEED_INCLUDE_DIR MINISEED_LIBRARY)
mark_as_advanced(MINISEED_INCLUDE_DIR MINISEED_LIBRARY)
