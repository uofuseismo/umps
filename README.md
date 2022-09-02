# About 

[Utah Message Passing System](https://uofuseismo.github.io/umps/) is a network-seismology-oriented message passing library that

   1. Allows for multiple communication patterns - e.g., pub/sub, request/reply between different computers on a network.
   2. Addresses the network exploration problem with a hub and spoke model.
   3. Can validate connections.

# Compiling

Currently, the software has only been used on Ubuntu20.  I anticipate there will be no issues for using later flavors of Ubuntu.  However, you may find it difficult to switch to other operating systems since they may have minimal to non-existant C++20 support.  For these instances I recommend installing and using the latest LLVM compiler suite.

Regardless, prior to attempting to compile the software you must clear the following dependencies

## Prerequisites

The following libraries are required to build the core software

   1.  C++20.  Yep, compiled languages just won't die.
   2.  [CMake](cmake.org) to generate makefiles.
   3.  [Boost](https://www.boost.org/) is a set of C++ portable libraries.
   3.  [libsodium](https://github.com/jedisct1/libsodium) for cryptographic utilities used in ZeroMQ and the authenticator.
   4.  [libsqlite3](https://www.sqlite.org/index.html) provides simple database functionality.  For example, the base authenticator uses it to store keys and passwords.
   5.  [ZeroMQ](https://zeromq.org/) v4 is, loosely speaking, a networking library.  ZeroMQ must be compiled with [libsodium](https://github.com/jedisct1/libsodium).  You may be thinking - Earthworm can send messages to and from computers.  That's absolutely true and UMPS can even ingest and produce some Earthworm messages.   Unfortunately, Earthworm's messaging system was never designed to be used as a library.  ZeroMQ was.  ZeroMQ is also provably scalable, actively maintained by a much broader community, has terrific documentation, a very nice API, and can perform user authentication.  It's also dead easy to install or build from source.  
   6.  [cppzmq](https://github.com/zeromq/cppzmq) the header-only C++ interface to ZeroMQ.
   7.  [spdlog](https://github.com/gabime/spdlog) underlies the default logger.  Again, I know, Earthworm can log messages (logit).  But, again, spdlog was built exclusively for logging messages.  Technically, if you really want to use logit then you would simply extend the base class.
   8.  [GTest](https://github.com/google/googletest) for unit testing.

Ideas on how to obtain these can be gleaned scripts/Dockererfile.ubuntu22.

## Optional

   1.  [pybind11](https://github.com/pybind/pybind11) is used to generate Python bindings.
   2.  [Doxygen](https://www.doxygen.nl/index.html) for generating API documentation.
   3.  [Earthworm](http://folkworm.ceri.memphis.edu/ew-dist/v7.10.1/earthworm_7.10.1-src.tgz) v7.10 or greater for communicating with other earthworm rings.  Note, when compiling you must compile with an option to create position independent code.  For example, on GCC you would add the -fPIC flag.

## Configuration

After the prerequisites have been installed you can attempt to configure the software.  For example, I would use the following to configure a GCC build that does not use Earthworm or Python

    #!/bin/bash
    export CXX=g++
    export BUILD_DIR=gcc_build
    if [ -d ${BUILD_DIR} ]; then
       rm -rf ${BUILD_DIR}
    fi
    mkdir ${BUILD_DIR}
    cd ${BUILD_DIR}
    cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=${CXX} \
    -DCMAKE_CXX_FLAGS="-Wall -O2"

For those using earthworm and Python, the following script may be of interest

    #!/bin/bash
    export CXX=clang++
    export BUILD_DIR=clang_build
    export EARTHWORM_ROOT=/opt/earthworm/earthworm_7.10/
    if [ -d ${BUILD_DIR} ]; then
       rm -rf ${BUILD_DIR}
    fi
    mkdir ${BUILD_DIR}
    cd ${BUILD_DIR}
    cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=${CXX} \
    -DCMAKE_CXX_FLAGS="-Wall" \
    -DWRAP_PYTHON=ON \
    -DPYTHON_EXECUTABLE=${HOME}/anaconda3/bin/python \
    -DPYTHON_LIBRARIES=${HOME}/anaconda3/lib/libpython3.8.so


## Building, Testing, Installing

Provided the configuration was successful, then descend into the appropriate build directory, say gcc\_build, and compile

    cd gcc_build
    make

The unit tests can then be run

    make test

The documentation can be generated, provided Doxygen was found, by typing

    make docs

Finally, the software may be installed 

    make install

This may require sudo.
