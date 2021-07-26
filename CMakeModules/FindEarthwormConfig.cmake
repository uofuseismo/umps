# Already in cache, be silent
if (EARTHWORM_INCLUDE_DIR AND EARTHWORM_LIBRARY)
   set (EARTHWORM_FIND_QUIETLY TRUE)
endif()

find_path(EARTHWORM_INCLUDE_DIR
          NAMES earthworm.h
          HINTS $ENV{EARTHWORM_ROOT}/include
                /opt/intel/tbb/include
                /usr/local/include
                /usr/include)
find_library(EARTHWORM_MT_LIBRARY
             NAMES ew_mt
             PATHS $ENV{EARTHWORM_ROOT}/lib
                   $ENV{EARTHWORM_ROOT}/lib64
                   /usr/local/lib
                   /usr/local/lib64)
find_library(EARTHWORM_UTILITY_LIBRARY
             names ew_util
             PATHS $ENV{EARTHWORM_ROOT}/lib
                   $ENV{EARTHWORM_ROOT}/lib64
                   /usr/local/lib
                   /usr/local/lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FindEarthworm DEFAULT_MSG EARTHWORM_INCLUDE_DIR EARTHWORM_UTILITY_LIBRARY EARTHWORM_MT_LIBRARY)
mark_as_advanced(EARTHWORM_INCLUDE_DIR EARTHWORM_UTILITY_LIBRARY EARTHWORM_MT_LIBRARY)
