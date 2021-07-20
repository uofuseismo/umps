# Already in cache, be silent
if (ZeroMQ_INCLUDE_DIR AND ZeroMQ_LIBRARY)
   set (ZeroMQ_FIND_QUIETLY TRUE)
endif()

find_path(ZeroMQ_INCLUDE_DIR
          NAMES zmq.h
          HINTS $ENV{ZeroMQ_ROOT}/include
                /opt/intel/tbb/include
                /usr/local/include
                /usr/include)
find_library(ZeroMQ_LIBRARY
             NAMES zmq
             PATHS $ENV{ZeroMQ_ROOT}/lib
                   $ENV{ZeroMQ_ROOT}/lib64
                   /usr/local/lib
                   /usr/local/lib64)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FindZeroMQ DEFAULT_MSG ZeroMQ_INCLUDE_DIR ZeroMQ_LIBRARY)
mark_as_advanced(ZeroMQ_INCLUDE_DIR ZeroMQ_LIBRARY)
