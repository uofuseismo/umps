cmake_minimum_required(VERSION 3.16)
project(URTS VERSION 0.1.0 LANGUAGES CXX)
enable_testing()

# Some options
option(EARTHWORM_TRACEBUF2_SIZE "TraceBuf2 message size in bytes" 4096)

# Ensure we have necessary packages 
include(CheckCXXCompilerFlag)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)

set(FindZeroMQ_DIR ${CMAKE_SOURCE_DIR}/CMakeModules) 
set(FindTime_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindEarthworm_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
find_package(FindZeroMQ REQUIRED)
find_package(FindTime REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(FindEarthworm)

# Versioning information
configure_file(${CMAKE_SOURCE_DIR}/include/private/version.hpp.in
               ${CMAKE_SOURCE_DIR}/include/urts/version.hpp)


include_directories(
   ${CMAKE_SOURCE_DIR}/include
   ${TIME_INCLUDE_DIR}
)

set(BUILD_EW FALSE)
if (${FindEarthworm_FOUND})
   message("Using TraceBuf2 message size of:" ${EARTHWORM_TRACEBUF2_SIZE})
   set(BUILD_EW TRUE)
   add_compile_definitions(WITH_EARTHWORM)
endif()

set(PUBLIC_HEADER_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include/urts)
set(PRIVATE_HEADER_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include/private)


set(LOGGING_SRC
    src/logging/stdout.cpp
    src/logging/spdlog.cpp)
set(MESSAGE_FORMAT_SRC
    src/messageFormats/earthworm/traceBuf2.cpp)
set(MESSAGING_SRC
    src/messaging/publisherSubscriber/publisher.cpp
    src/messaging/publisherSubscriber/subscriber.cpp)
if (${BUILD_EW})
   set(EARTHWORM_SRC 
       src/messaging/earthworm/waveRing.cpp)
endif()

set(BUILD_SHARED_LIBS YES)
add_library(urts ${LOGGING_SRC} ${MESSAGE_FORMAT_SRC} ${MESSAGING_SRC} ${EARTHWORM_SRC})
set_target_properties(urts PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(urts
                      PUBLIC ${TIME_LIBRARY} ${ZeroMQ_LIBRARY}
                      PRIVATE spdlog::spdlog nlohmann_json::nlohmann_json)
target_include_directories(urts
                           PRIVATE spdlog::spdlog ${ZeroMQ_INCLUDE_DIR}
                           PUBLIC $<BUILD_INTERFACE:${PUBLIC_HEADER_DIRECTORIES}>
                           PUBLIC $<INSTALL_INTERFACE:${PUBLIC_HEADER_DIRECTORIES})
if (${BUILD_EW})
    target_include_directories(urts PRIVATE ${EARTHWORM_INCLUDE_DIR})
    target_link_libraries(urts PRIVATE ${EARTHWORM_UTILITY_LIBRARY} ${EARTHWORM_MT_LIBRARY})
endif()

##########################################################################################
#                                      Python Wrapping                                   #
##########################################################################################
if (WRAP_PYTHON)
   find_package(pybind11 REQUIRED)
   #add_library(pytime MODULE
   #            python/pyurts.cpp)
   #target_link_libraries(pytime PRIVATE pybind11::module urts)
   #target_include_directories(pytime
   #                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/python>
   #                           PRIVATE $<BUILD_INTERFACE:${PUBLIC_HEADER_DIRECTORIES})
   #set_target_properties(pyurts PROPERTIES
   #                      PREFIX ""
   #                      CXX_STANDARD 20
   #                      CXX_STANDARD_REQUIRED YES
   #                      CXX_EXTENSIONS NO)
endif()

##########################################################################################
#                                         Unit Tests                                     #
##########################################################################################
set(TEST_SRC
    testing/main.cpp
    testing/messageFormats/earthworm.cpp
    testing/messaging/pubsub.cpp)
add_executable(unitTests ${TEST_SRC})
set_target_properties(unitTests PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_link_libraries(unitTests PRIVATE urts ${GTEST_BOTH_LIBRARIES})
target_include_directories(unitTests
                           PRIVATE ${GTEST_INCLUDE_DIRS}
                           PUBLIC $<BUILD_INTERFACE:${PUBLIC_HEADER_DIRECTORIES}>)
add_test(NAME unitsTests
         COMMAND unitTests)

if (WRAP_PYTHON)
#   file(COPY ${CMAKE_SOURCE_DIR}/python/unit_test.py DESTINATION .)
#   add_test(NAME python_tests
#            COMMAND ${PYTHON_EXECUTABLE} -m pytest)
endif()

##########################################################################################
#                                         Installation                                   #
##########################################################################################
include(GNUInstallDirs)
if (WRAP_PYTHON)
   install(TARGETS urts #pyurts
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else()
   install(TARGETS urts
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
##########################################################################################
#                                     CPACK Packaging                                    #
##########################################################################################
set(CPACK_PACKAGE_NAME "URTS")
set(CPACK_PACKAGE_VENDOR "UUSS")
set(CPACK_PACKAGE_CONTACT "bbaker@seis.utah.edu")
set(CPACK_PACKAGE_LICENSE "MIT")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "UUSS's Real-Time Seismic System tools.")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
