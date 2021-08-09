# Already in cache, be silent
if (TBB_INCLUDE_DIR AND TBB_LIBRARY)
   set (TBB_FIND_QUIETLY TRUE)
endif()

find_path(TBB_INCLUDE_DIR
          NAMES tbb
          HINTS $ENV{TBB_ROOT}/include
                /opt/intel/oneapi/tbb/latest/include/oneapi
                /opt/intel/tbb/include
                /usr/local/include
                /usr/include)
find_library(TBB_LIBRARY
             NAMES tbb
             PATHS $ENV{TBB_ROOT}/lib
                   $ENV{TBB_ROOT}/lib/intel64/gcc4.8
                   $ENV{TBB_ROOT}/lib/intel64/gcc4.7
                   $ENV{TBB_ROOT}/lib64
                   /opt/intel/oneapi/tbb/latest/lib/intel64/gcc4.8
                   /opt/intel/tbb/lib/intel64/gcc4.7
                   /opt/intel/tbb/lib
                   /usr/local/lib
                   /usr/local/lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FindTBB DEFAULT_MSG TBB_INCLUDE_DIR TBB_LIBRARY)
mark_as_advanced(TBB_INCLUDE_DIR TBB_LIBRARY)

