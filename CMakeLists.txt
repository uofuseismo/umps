cmake_minimum_required(VERSION 3.16)
project(URTS VERSION 0.1.0 LANGUAGES CXX)
enable_testing()

# Some options
option(EARTHWORM_TRACEBUF2_SIZE "TraceBuf2 message size in bytes" 4096)

# Ensure we have necessary packages 
include(CheckCXXCompilerFlag)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(cppzmq REQUIRED)

set(FindZeroMQ_DIR ${CMAKE_SOURCE_DIR}/CMakeModules) 
set(FindTime_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindEarthworm_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindTBB_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindMiniSEED_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
find_package(FindZeroMQ REQUIRED)
find_package(FindTime REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(FindEarthworm)
find_package(FindTBB)
find_package(FindMiniSEED)

# Versioning information
configure_file(${CMAKE_SOURCE_DIR}/include/private/version.hpp.in
               ${CMAKE_SOURCE_DIR}/include/urts/version.hpp)


include_directories(
   ${CMAKE_SOURCE_DIR}/include
   ${TIME_INCLUDE_DIR}
)

set(BUILD_EW FALSE)
set(BUILD_MSEED FALSE)
if (${FindEarthworm_FOUND})
   message("Will compile Earthworm")
   set(BUILD_EW TRUE)
   add_compile_definitions(WITH_EARTHWORM)
   message("Using TraceBuf2 message size of:" ${EARTHWORM_TRACEBUF2_SIZE})
   if (${FindMiniSEED_FOUND})
      message("Will compile earthworm modules with MiniSEED")
      set(BUILD_MSEED TRUE)
      add_compile_definitions(WITH_MSEED)
   endif()
endif()
if (${FindTBB_FOUND})
   message("Building with TBB")
   set(BUILD_TBB TRUE)
   add_compile_definitions(WITH_TBB)
endif() 

set(PUBLIC_HEADER_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include/urts)
set(PRIVATE_HEADER_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/include/private)


set(LOGGING_SRC
    src/logging/stdout.cpp
    src/logging/spdlog.cpp)
set(MESSAGE_FORMAT_SRC
    src/messageFormats/dataPacket.cpp
    src/messageFormats/pick.cpp
    src/messageFormats/earthworm/traceBuf2.cpp)
set(MESSAGING_SRC
    src/messaging/authentication/certificate.cpp
    #src/messaging/authentication/generateKeyPair.cpp
    src/messaging/publisherSubscriber/publisher.cpp
    src/messaging/publisherSubscriber/subscriber.cpp
    src/messaging/publisherSubscriber/proxy.cpp
    src/messaging/requestRouter/request.cpp
    src/messaging/requestRouter/router.cpp)
set(MODULES_LIBSRC
    src/services/incrementer/request.cpp
    src/services/incrementer/response.cpp
    src/services/incrementer/counter.cpp
    src/services/incrementer/parameters.cpp
    src/services/incrementer/service.cpp
    src/services/packetCache/circularBuffer.cpp
    src/services/packetCache/cappedCollection.cpp
    src/services/packetCache/dataRequest.cpp)
if (${BUILD_EW})
   set(EARTHWORM_SRC 
       src/messaging/earthworm/waveRing.cpp)
endif()

set(BUILD_SHARED_LIBS YES)
add_library(urts ${LOGGING_SRC} ${MESSAGE_FORMAT_SRC} ${MESSAGING_SRC}
                 ${MODULES_LIBSRC} ${EARTHWORM_SRC})
set_target_properties(urts PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(urts
                      PUBLIC ${TIME_LIBRARY} cppzmq #${ZeroMQ_LIBRARY}
                      PRIVATE spdlog::spdlog nlohmann_json::nlohmann_json)
target_include_directories(urts
                           PRIVATE spdlog::spdlog ${cppzmq_INCLUDE_DIR} #cppzmq #${ZeroMQ_INCLUDE_DIR}
                           PUBLIC $<BUILD_INTERFACE:${PUBLIC_HEADER_DIRECTORIES}>
                           PUBLIC $<INSTALL_INTERFACE:${PUBLIC_HEADER_DIRECTORIES})
if (${BUILD_TBB})
    target_include_directories(urts PRIVATE ${TBB_INCLUDE_DIR})
    target_link_libraries(urts PRIVATE ${TBB_LIBRARY})
endif()
if (${BUILD_EW})
    target_include_directories(urts PRIVATE ${EARTHWORM_INCLUDE_DIR})
    target_link_libraries(urts PRIVATE ${EARTHWORM_UTILITY_LIBRARY} ${EARTHWORM_MT_LIBRARY})
    if (${BUILD_MSEED})
       target_include_directories(urts PRIVATE ${MINISEED_INCLUDE_DIR})
       target_link_libraries(urts PRIVATE ${MINISEED_LIBRARY})
    endif()
endif()

##########################################################################################
#                                         Modules                                        #
##########################################################################################

add_executable(uOperator src/modules/operator/operator.cpp)
set_target_properties(uOperator PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(uOperator PRIVATE urts Boost::program_options pthread) 
if (${BUILD_EW})
    add_executable(packetCache src/services/packetCache/packetCache.cpp)
    set_target_properties(packetCache PROPERTIES
                          CXX_STANDARD 20
                          CXX_STANDARD_REQUIRED YES
                          CXX_EXTENSIONS NO) 
    target_link_libraries(packetCache PRIVATE urts Boost::program_options pthread)
    #target_include_directories(packetCache PRIVATE ${Boost_INCLUDE_DIR})
endif()

add_executable(uKeyPair src/modules/keypair/keypair.cpp)
set_target_properties(uKeyPair PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(uKeyPair PRIVATE Boost::program_options urts)


##########################################################################################
#                                      Python Wrapping                                   #
##########################################################################################
if (WRAP_PYTHON)
   find_package(pybind11 REQUIRED)
   add_library(pyurts MODULE
               python/pyurts.cpp
               python/messageFormats/message.cpp
               python/messageFormats/dataPacket.cpp
               python/messageFormats/pick.cpp
               python/messaging/publisherSubscriber/publisher.cpp
               python/messaging/publisherSubscriber/subscriber.cpp)
   target_link_libraries(pyurts PRIVATE pybind11::module urts)
   target_include_directories(pyurts
                              PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/python/include>
                                      $<BUILD_INTERFACE:${PUBLIC_HEADER_DIRECTORIES}>)
   set_target_properties(pyurts PROPERTIES
                         PREFIX ""
                         CXX_STANDARD 20
                         CXX_STANDARD_REQUIRED YES
                         CXX_EXTENSIONS NO)
endif()

##########################################################################################
#                                         Unit Tests                                     #
##########################################################################################
set(TEST_SRC
    testing/main.cpp
    testing/messageFormats/pick.cpp
    testing/messageFormats/earthworm.cpp
    testing/messageFormats/dataPacket.cpp
    testing/modules/packetCache.cpp
    testing/modules/incrementer.cpp
    testing/messaging/authentication.cpp
    testing/messaging/pubsub.cpp
    testing/messaging/requestRouter.cpp
    testing/messaging/xpubxsub.cpp
    )
add_executable(unitTests ${TEST_SRC})
set_target_properties(unitTests PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_link_libraries(unitTests PRIVATE urts ${GTEST_BOTH_LIBRARIES} pthread)
target_include_directories(unitTests
                           PRIVATE ${GTEST_INCLUDE_DIRS}
                           PUBLIC $<BUILD_INTERFACE:${PUBLIC_HEADER_DIRECTORIES}>)
add_test(NAME unitTests
         COMMAND unitTests)

if (WRAP_PYTHON)
   file(COPY ${CMAKE_SOURCE_DIR}/python/unit_test.py DESTINATION .)
   add_test(NAME python_tests
            COMMAND ${PYTHON_EXECUTABLE} -m pytest)
endif()

##########################################################################################
#                                         Installation                                   #
##########################################################################################
include(GNUInstallDirs)
if (WRAP_PYTHON)
   install(TARGETS urts pyurts
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
