cmake_minimum_required(VERSION 3.16)
project(UMPS VERSION 0.1.0 LANGUAGES CXX)
enable_testing()

# Some options
option(EARTHWORM_DEFAULT_TRACEBUF2_SIZE "TraceBuf2 message size in bytes" ON)
option(WRAP_PYTHON "Compile the Python bindings" OFF)

# Ensure we have necessary packages 
include(CheckCXXCompilerFlag)
find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(cppzmq REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Doxygen)

set(FindZeroMQ_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
#set(FindTime_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindEarthworm_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindTBB_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
set(FindMiniSEED_DIR ${CMAKE_SOURCE_DIR}/CMakeModules)
find_package(FindZeroMQ REQUIRED)
#find_package(FindTime REQUIRED)
find_package(FindEarthworm)
if (${FindEarthworm_FOUND})
   find_package(FindMiniSEED)
endif()
find_package(FindTBB)

# Versioning information
configure_file(${CMAKE_SOURCE_DIR}/include/private/version.hpp.in
               ${CMAKE_SOURCE_DIR}/include/umps/version.hpp)


#include_directories(
#   ${CMAKE_SOURCE_DIR}/include
#   #${TIME_INCLUDE_DIR}
#)

set(BUILD_EW FALSE)
set(BUILD_MSEED FALSE)
if (${FindEarthworm_FOUND})
   message("Will compile Earthworm")
   set(BUILD_EW TRUE)
   if (${EARTHWORM_DEFAULT_TRACEBUF2_SIZE})
      set(EARTHWORM_TRACEBUF2_SIZE 4096)
   endif()
   message("Using TraceBuf2 message size of: " ${EARTHWORM_TRACEBUF2_SIZE})
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

set(VERSION_SRC
    src/version.cpp)
set(LOGGING_SRC
    src/logging/stdout.cpp
    src/logging/spdlog.cpp)
set(MESSAGE_FORMAT_SRC
    src/messageFormats/messages.cpp
    src/messageFormats/dataPacket.cpp
    src/messageFormats/pick.cpp
    src/messageFormats/text.cpp)
set(MESSAGING_SRC
    src/authentication/authenticator.cpp
    src/authentication/service.cpp
    src/authentication/sqlite3Authenticator.cpp
    src/authentication/grasslands.cpp
    src/authentication/user.cpp
    src/authentication/zapOptions.cpp
    src/authentication/certificate/keys.cpp
    src/authentication/certificate/userNameAndPassword.cpp
    #src/authentication/generateKeyPair.cpp
    src/messaging/context.cpp
    src/messaging/publisherSubscriber/publisher.cpp
    src/messaging/publisherSubscriber/publisherOptions.cpp
    src/messaging/publisherSubscriber/subscriber.cpp
    src/messaging/publisherSubscriber/subscriberOptions.cpp
    src/messaging/requestRouter/request.cpp
    src/messaging/requestRouter/requestOptions.cpp
    src/messaging/requestRouter/router.cpp
    src/messaging/requestRouter/routerOptions.cpp
    src/messaging/routerDealer/proxy.cpp
    src/messaging/routerDealer/proxyOptions.cpp
    src/messaging/routerDealer/request.cpp
    src/messaging/routerDealer/requestOptions.cpp
    src/messaging/routerDealer/reply.cpp
    src/messaging/routerDealer/replyOptions.cpp
    src/messaging/xPublisherXSubscriber/proxy.cpp
    src/messaging/xPublisherXSubscriber/proxyOptions.cpp
    src/messaging/xPublisherXSubscriber/publisher.cpp
    src/messaging/xPublisherXSubscriber/publisherOptions.cpp)
set(MODULES_LIBSRC
    src/modules/process.cpp
    src/modules/processManager.cpp
    #src/modules/module.cpp
    src/services/command/availableCommandsRequest.cpp
    src/services/command/availableCommandsResponse.cpp
    src/services/command/commandRequest.cpp
    src/services/command/commandResponse.cpp
    src/services/command/localModuleDetails.cpp
    src/services/command/localModuleTable.cpp
    src/services/command/localRequestor.cpp
    src/services/command/localRequestorOptions.cpp
    src/services/command/localService.cpp
    src/services/command/localServiceOptions.cpp
    #src/services/connectionInformation/availableBroadcastsRequest.cpp
    #src/services/connectionInformation/availableBroadcastsResponse.cpp
    src/services/connectionInformation/availableConnectionsRequest.cpp
    src/services/connectionInformation/availableConnectionsResponse.cpp
    #src/services/connectionInformation/getConnections.cpp
    src/services/connectionInformation/requestorOptions.cpp
    src/services/connectionInformation/requestor.cpp
    src/services/connectionInformation/service.cpp
    src/services/connectionInformation/serviceOptions.cpp
    src/services/connectionInformation/socketDetails/dealer.cpp
    src/services/connectionInformation/socketDetails/proxy.cpp
    src/services/connectionInformation/socketDetails/publisher.cpp
    src/services/connectionInformation/socketDetails/request.cpp
    src/services/connectionInformation/socketDetails/reply.cpp
    src/services/connectionInformation/socketDetails/router.cpp
    src/services/connectionInformation/socketDetails/subscriber.cpp
    src/services/connectionInformation/socketDetails/xPublisher.cpp
    src/services/connectionInformation/socketDetails/xSubscriber.cpp
    src/services/connectionInformation/details.cpp
    src/services/moduleRegistry/moduleDetails.cpp
    src/services/moduleRegistry/serviceOptions.cpp
    src/services/moduleRegistry/registeredModulesRequest.cpp
    src/services/moduleRegistry/registeredModulesResponse.cpp
    src/services/moduleRegistry/registrationRequest.cpp
    src/services/moduleRegistry/registrationResponse.cpp
    src/services/moduleRegistry/service.cpp
    src/proxyBroadcasts/proxy.cpp
    src/proxyBroadcasts/proxyOptions.cpp
    #src/proxyBroadcasts/dataPacket/proxy.cpp
    #src/proxyBroadcasts/dataPacket/proxyOptions.cpp
    src/proxyBroadcasts/dataPacket/publisher.cpp
    src/proxyBroadcasts/dataPacket/publisherOptions.cpp
    src/proxyBroadcasts/dataPacket/subscriber.cpp
    src/proxyBroadcasts/dataPacket/subscriberOptions.cpp
    #src/proxyBroadcasts/heartbeat/proxy.cpp
    #src/proxyBroadcasts/heartbeat/proxyOptions.cpp
    src/proxyBroadcasts/heartbeat/publisher.cpp
    src/proxyBroadcasts/heartbeat/publisherOptions.cpp
    src/proxyBroadcasts/heartbeat/publisherProcessOptions.cpp
    src/proxyBroadcasts/heartbeat/publisherProcess.cpp
    src/proxyBroadcasts/heartbeat/subscriber.cpp
    src/proxyBroadcasts/heartbeat/subscriberOptions.cpp
    src/proxyBroadcasts/heartbeat/status.cpp
    src/proxyBroadcasts/pick/publisher.cpp
    src/proxyBroadcasts/pick/publisherOptions.cpp
    src/proxyBroadcasts/pick/subscriber.cpp
    src/proxyBroadcasts/pick/subscriberOptions.cpp
    src/proxyServices/proxy.cpp
    src/proxyServices/proxyOptions.cpp
    src/proxyServices/incrementer/counter.cpp
    src/proxyServices/incrementer/incrementRequest.cpp
    src/proxyServices/incrementer/incrementResponse.cpp
    src/proxyServices/incrementer/itemsRequest.cpp
    src/proxyServices/incrementer/itemsResponse.cpp
    src/proxyServices/incrementer/options.cpp
    src/proxyServices/incrementer/requestor.cpp
    src/proxyServices/incrementer/requestorOptions.cpp
    src/proxyServices/incrementer/replier.cpp
    src/proxyServices/incrementer/replierOptions.cpp
    src/proxyServices/incrementer/service.cpp
    src/proxyServices/packetCache/bulkDataRequest.cpp
    src/proxyServices/packetCache/bulkDataResponse.cpp
    src/proxyServices/packetCache/circularBuffer.cpp
    src/proxyServices/packetCache/cappedCollection.cpp
    src/proxyServices/packetCache/dataRequest.cpp
    src/proxyServices/packetCache/dataResponse.cpp
    src/proxyServices/packetCache/wigginsInterpolator.cpp
    #src/proxyServices/packetCache/proxy.cpp
    #src/proxyServices/packetCache/proxyOptions.cpp
    src/proxyServices/packetCache/sensorRequest.cpp
    src/proxyServices/packetCache/sensorResponse.cpp
    src/proxyServices/packetCache/service.cpp
    src/proxyServices/packetCache/replier.cpp
    src/proxyServices/packetCache/replierOptions.cpp
    src/proxyServices/packetCache/requestor.cpp
    src/proxyServices/packetCache/requestorOptions.cpp
    src/modules/operator/readZAPOptions.cpp)
if (${BUILD_EW})
   set(EARTHWORM_SRC 
       src/earthworm/traceBuf2.cpp
       src/earthworm/waveRing.cpp
       src/logging/logit.cpp
       )
endif()

set(BUILD_SHARED_LIBS YES)
add_library(umps ${VERSION_SRC} ${LOGGING_SRC} ${MESSAGING_SRC}
                 ${MESSAGE_FORMAT_SRC}
                 ${MODULES_LIBSRC} ${EARTHWORM_SRC})
set_target_properties(umps PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(umps
                      PUBLIC cppzmq Threads::Threads
                      PRIVATE spdlog::spdlog nlohmann_json::nlohmann_json ${SQLite3_LIBRARIES})
target_include_directories(umps
                           PRIVATE spdlog::spdlog
                           PRIVATE ${cppzmq_INCLUDE_DIR}
                           PRIVATE ${SQLite3_INCLUDE_DIRS}
                           PRIVATE Boost::program_options
                           PUBLIC $<INSTALL_INTERFACE:${CMAKE_SOURCE_DIR}/include/umps>
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
if (${BUILD_TBB})
    target_include_directories(umps PRIVATE ${TBB_INCLUDE_DIR})
    target_link_libraries(umps PRIVATE ${TBB_LIBRARY})
endif()
if (${BUILD_EW})
    target_include_directories(umps PRIVATE ${EARTHWORM_INCLUDE_DIR})
    target_link_libraries(umps PRIVATE ${EARTHWORM_UTILITY_LIBRARY} ${EARTHWORM_MT_LIBRARY})
    target_compile_definitions(umps PRIVATE WITH_EARTHWORM=1)
    if (${BUILD_MSEED})
       target_include_directories(umps PRIVATE ${MINISEED_INCLUDE_DIR})
       target_link_libraries(umps PRIVATE ${MINISEED_LIBRARY})
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
target_include_directories(uOperator
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(uOperator PRIVATE umps Boost::program_options Threads::Threads)

add_executable(packetCache src/modules/services/packetCache.cpp)
set_target_properties(packetCache PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(packetCache
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(packetCache PRIVATE umps Boost::program_options Threads::Threads)


add_executable(incrementer src/modules/services/incrementer.cpp)
set_target_properties(incrementer PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(incrementer
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(incrementer PRIVATE umps Boost::program_options Threads::Threads)

add_executable(uKeyPair src/modules/authentication/keypair.cpp)
set_target_properties(uKeyPair PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(uKeyPair
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(uKeyPair PRIVATE Boost::program_options umps)

add_executable(uUserTable src/modules/authentication/userTable.cpp)
set_target_properties(uUserTable PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_include_directories(uUserTable
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(uUserTable PRIVATE Boost::program_options umps)

add_executable(uCommand src/modules/uCommand.cpp)
set_target_properties(uCommand PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(uCommand
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(uCommand PRIVATE Boost::program_options umps)

if (${BUILD_EW})
    add_executable(broadcastWaveRing src/modules/broadcasts/waveRing.cpp)
    set_target_properties(broadcastWaveRing PROPERTIES
                          CXX_STANDARD 20
                          CXX_STANDARD_REQUIRED YES 
                          CXX_EXTENSIONS NO) 
    target_include_directories(broadcastWaveRing
                               PRIVATE ${CMAKE_SOURCE_DIR}/include
                               PRIVATE Boost::program_options)

    target_link_libraries(broadcastWaveRing PRIVATE umps Boost::program_options Threads::Threads)
endif()

##########################################################################################
#                                      Python Wrapping                                   #
##########################################################################################
if (WRAP_PYTHON)
   message("Will build Python bindings")
   find_package(pybind11 REQUIRED)
   add_library(pyumps MODULE
               python/pyumps.cpp
               python/messageFormats/message.cpp
               python/messageFormats/messages.cpp
               python/messageFormats/dataPacket.cpp
               python/messageFormats/pick.cpp
               python/authentication/keys.cpp
               python/authentication/zapOptions.cpp
               python/proxyBroadcasts/dataPacketSubscriber.cpp
               python/proxyBroadcasts/dataPacketSubscriberOptions.cpp
               python/proxyBroadcasts/heartbeat.cpp
               python/messaging/publisherSubscriber/publisher.cpp
               python/messaging/publisherSubscriber/subscriberOptions.cpp
               python/messaging/publisherSubscriber/subscriber.cpp)
   target_link_libraries(pyumps PRIVATE pybind11::module umps)
   target_include_directories(pyumps
                              PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/python/include>
                                      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
   set_target_properties(pyumps PROPERTIES
                         PREFIX ""
                         CXX_STANDARD 20
                         CXX_STANDARD_REQUIRED YES
                         CXX_EXTENSIONS NO)
endif()

##########################################################################################
#                                         Unit Tests                                     #
##########################################################################################
file(COPY ${CMAKE_SOURCE_DIR}/testing/data DESTINATION .)
set(TEST_SRC
    testing/main.cpp
    testing/messageFormats/messages.cpp
    testing/messageFormats/heartbeat.cpp
    testing/messageFormats/pick.cpp
    testing/messageFormats/dataPacket.cpp
    testing/messageFormats/text.cpp
    testing/messageFormats/heartbeat.cpp
    testing/broadcasts/proxyOptions.cpp
    testing/broadcasts/dataPacket.cpp
    testing/broadcasts/heartbeat.cpp
    testing/broadcasts/pick.cpp
    testing/services/proxyOptions.cpp
    testing/services/packetCache.cpp
    testing/services/incrementer.cpp
    testing/services/command.cpp
    testing/services/connectionInformation.cpp
    testing/services/moduleRegistry.cpp
    testing/messaging/authentication.cpp
    testing/messaging/options.cpp
    )
if (${BUILD_EW})
   set(TEST_SRC ${TEST_SRC} testing/messageFormats/earthworm.cpp)
endif()
add_executable(unitTests ${TEST_SRC})
set_target_properties(unitTests PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES 
                      CXX_EXTENSIONS NO) 
target_link_libraries(unitTests PRIVATE umps ${GTEST_BOTH_LIBRARIES})
target_include_directories(unitTests
                           PRIVATE ${GTEST_INCLUDE_DIRS}
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

# Actually test communication
set(TEST_COMMUNICATION_SRC
    testing/communication/main.cpp
    testing/communication/pubsub.cpp
    testing/communication/authentication.cpp
    testing/communication/requestRouter.cpp
    testing/communication/xpubxsub.cpp
    testing/communication/routerDealer.cpp
    testing/communication/dataPacket.cpp
    testing/communication/packetCache.cpp
    testing/communication/incrementer.cpp)
add_executable(commTests ${TEST_COMMUNICATION_SRC})
set_target_properties(commTests PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_link_libraries(commTests PRIVATE umps ${GTEST_BOTH_LIBRARIES})
target_include_directories(commTests
                           PRIVATE ${GTEST_INCLUDE_DIRS}
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

# Add the tests
add_test(NAME unitTests
         COMMAND unitTests)
add_test(NAME commTests
         COMMAND commTests)

if (WRAP_PYTHON)
   file(COPY ${CMAKE_SOURCE_DIR}/python/unit_test.py DESTINATION .)
   add_test(NAME python_tests
            COMMAND ${PYTHON_EXECUTABLE} -m pytest)
endif()

##########################################################################################
#                                           Doxygen                                      #
##########################################################################################
if (DOXYGEN_FOUND)
   # set input and output files
   set(DOXYGEN_IN  ${CMAKE_CURRENT_SOURCE_DIR}/docs/docs_doxygen/Doxyfile.in)
   set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)
   # request to configure the file
   configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
   message("Doxygen build started")
   file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/docs/docs_doxygen/umps_layout.xml
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
   # Note: do not put "ALL" - this builds docs together with application EVERY TIME!
   add_custom_target(docs
                     COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                     COMMENT "Generating API documentation with Doxygen"
                     VERBATIM)
else (DOXYGEN_FOUND)
   message("Doxygen need to be installed to generate the doxygen documentation")
endif()
##########################################################################################
#                                         Installation                                   #
##########################################################################################
include(GNUInstallDirs)
if (WRAP_PYTHON)
   install(TARGETS umps pyumps
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else()
   install(TARGETS umps
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/umps
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
##########################################################################################
#                                     CPACK Packaging                                    #
##########################################################################################
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "UUSS")
set(CPACK_PACKAGE_CONTACT "ben.baker@utah.edu")
set(CPACK_PACKAGE_LICENSE "MIT")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "University of Utah Seismograph Stations Messaging Passing System")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_COMPONENTS_ALL libraries headers)
if (WIN32)
    set(CPACK_GENERATOR ZIP WIX)
elseif (APPLE)
    set(CPACK_GENERATOR TGZ productbuild)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR TGZ RPM)
else()
    set(CPACK_GENERATOR TGZ)
endif()
set(CPACK_SOURCE_IGNORE_FILES
  /\\.git/
  \\.swp
  \\.orig
  /CMakeLists\\.txt\\.user
  /private/
)
include(CPack) # Put this last!
