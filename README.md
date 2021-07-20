# About 

Utah Real Time Seismology is a developmental system for complementing the traditional the Earthworm/AQMS system with machine learning tools.  URTS is extremely experimental and not recommended for general use. 

# Compiling

Prior to attempting to compile the software you must clear the following dependencies

## Prerequisites

The following libraries are required to build the core software

    1.  C++20.  Yep, compiled languages just won't die.
    2.  [CMake](cmake.org) to generate makefiles.
    3.  [Boost](https://www.boost.org/) is a set of C++ portable libraries.
    4.  [ZeroMQ](https://zeromq.org/) v3 is, loosely speaking, a networking library.  You may be thinking - Earthworm can send messages to and from computers.  That's absolutely true and URTS can even ingest and produce some Earthworm messages.   Unfortunately, Earthworm's messaging system was never designed to be used as a library.  ZeroMQ was.  ZeroMQ is also provably scalable, s actively maintained by a much broader community, has terrific documentation, and has a very nice API.  It's also dead easy to install or build from source.
    5.  [spdlog](https://github.com/gabime/spdlog) underlies the default logger.  Again, I know, Earthworm can log messages (logit).  But, again, spdlog was built exclusively for logging messages.  Technically, if you really want to use logit then you would simply extend the base class.
    6.  [GTest](https://github.com/google/googletest) for unit testing.
    7.  [Time](https://github.com/uofuseismo/time) this is a small library for performing epoch to UTC time conversions.

## Optional

    1.  [Earthworm](http://folkworm.ceri.memphis.edu/ew-dist/v7.10.1/earthworm_7.10.1-src.tgz) v7.10 or greater for communicating with other earthworm rings.  Note, when compiling you must compile with an option to create position independent code.  For example, on GCC you would add the -fPIC flag.
    2.  [pybind11](https://github.com/pybind/pybind11) is used to generate Python bindings.

## Configuration

After the 
