cmake_minimum_required(VERSION 3.16)
project(UMPS VERSION 0.3.0 LANGUAGES CXX)
enable_testing()

# Some options
option(WRAP_PYTHON "Compile the Python bindings" OFF)
option(BUILD_EXAMPLES "Compile the examples" OFF)

# Ensure we have necessary packages 
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
include(CheckCXXCompilerFlag)
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(cppzmq REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(cppzmq REQUIRED)
add_compile_definitions(UMPS_SRC)

# Versioning information
configure_file(${CMAKE_SOURCE_DIR}/include/private/version.hpp.in
               ${CMAKE_SOURCE_DIR}/include/umps/version.hpp)


set(VERSION_SRC
    src/version.cpp)
set(LOGGING_SRC
    src/logging/standardOut.cpp
    src/logging/dailyFile.cpp)
set(MESSAGE_FORMAT_SRC
    src/messageFormats/failure.cpp
    src/messageFormats/message.cpp
    src/messageFormats/messages.cpp
    #src/messageFormats/dataPacket.cpp
    #src/messageFormats/pick.cpp
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
    src/messaging/context.cpp
    src/messaging/socketOptions.cpp
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
    src/messaging/xPublisherXSubscriber/publisherOptions.cpp
    src/messaging/xPublisherXSubscriber/subscriber.cpp
    src/messaging/xPublisherXSubscriber/subscriberOptions.cpp)
set(MODULES_LIBSRC
    src/modules/process.cpp
    src/modules/processManager.cpp
    #src/modules/module.cpp
    src/services/command/availableCommandsRequest.cpp
    src/services/command/availableCommandsResponse.cpp
    src/services/command/commandRequest.cpp
    src/services/command/commandResponse.cpp
    src/services/command/moduleDetails.cpp
    src/services/command/moduleTable.cpp
    src/services/command/requestor.cpp
    src/services/command/requestorOptions.cpp
    src/services/command/service.cpp
    src/services/command/serviceOptions.cpp
    src/services/command/terminateRequest.cpp
    src/services/command/terminateResponse.cpp
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
    #src/services/moduleRegistry/moduleDetails.cpp
    #src/services/moduleRegistry/serviceOptions.cpp
    #src/services/moduleRegistry/registeredModulesRequest.cpp
    #src/services/moduleRegistry/registeredModulesResponse.cpp
    #src/services/moduleRegistry/registrationRequest.cpp
    #src/services/moduleRegistry/registrationResponse.cpp
    #src/services/moduleRegistry/service.cpp
    src/proxyBroadcasts/proxy.cpp
    src/proxyBroadcasts/proxyOptions.cpp
    src/proxyBroadcasts/heartbeat/publisher.cpp
    src/proxyBroadcasts/heartbeat/publisherOptions.cpp
    src/proxyBroadcasts/heartbeat/publisherProcessOptions.cpp
    src/proxyBroadcasts/heartbeat/publisherProcess.cpp
    src/proxyBroadcasts/heartbeat/subscriber.cpp
    src/proxyBroadcasts/heartbeat/subscriberOptions.cpp
    src/proxyBroadcasts/heartbeat/status.cpp
    src/proxyServices/proxy.cpp
    src/proxyServices/proxyOptions.cpp
    src/proxyServices/command/availableModulesRequest.cpp
    src/proxyServices/command/availableModulesResponse.cpp
    src/proxyServices/command/moduleDetails.cpp
    src/proxyServices/command/proxy.cpp
    src/proxyServices/command/proxyOptions.cpp
    src/proxyServices/command/registrationRequest.cpp
    src/proxyServices/command/registrationResponse.cpp
    src/proxyServices/command/requestor.cpp
    src/proxyServices/command/requestorOptions.cpp
    src/proxyServices/command/replier.cpp
    src/proxyServices/command/replierOptions.cpp
    src/proxyServices/command/replierProcess.cpp
    src/modules/operator/readZAPOptions.cpp)

set(BUILD_SHARED_LIBS YES)
add_library(umps ${VERSION_SRC} ${LOGGING_SRC} ${MESSAGING_SRC}
                 ${MESSAGE_FORMAT_SRC}
                 ${MODULES_LIBSRC} ${EARTHWORM_SRC})
set_target_properties(umps PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
message("dynamic linking " ${cppzmq_LIBRARY})
target_link_libraries(umps
                      PUBLIC libzmq Threads::Threads
                      PRIVATE spdlog::spdlog nlohmann_json::nlohmann_json ${SQLite3_LIBRARIES})
target_include_directories(umps
                           PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
                           PRIVATE spdlog::spdlog
                           PRIVATE ${cppzmq_INCLUDE_DIR}
                           PRIVATE ${SQLite3_INCLUDE_DIRS}
                           PRIVATE Boost::program_options
                           PUBLIC $<INSTALL_INTERFACE:${CMAKE_SOURCE_DIR}/include/umps>)

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

add_executable(uLocalCommand src/modules/uLocalCommand.cpp)
set_target_properties(uLocalCommand PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(uLocalCommand
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(uLocalCommand PRIVATE Boost::program_options umps)

add_executable(uRemoteCommand src/modules/uRemoteCommand.cpp)
set_target_properties(uRemoteCommand PROPERTIES
                      CXX_STANDARD 20
                      CXX_STANDARD_REQUIRED YES
                      CXX_EXTENSIONS NO)
target_include_directories(uRemoteCommand
                           PRIVATE ${CMAKE_SOURCE_DIR}/include
                           PRIVATE Boost::program_options)
target_link_libraries(uRemoteCommand PRIVATE Boost::program_options umps)

##########################################################################################
#                                      Python Wrapping                                   #
##########################################################################################
if (${WRAP_PYTHON})
   message("Will build Python bindings")
   find_package(pybind11 REQUIRED)
   add_library(umps_python
               python/authentication.cpp
               #python/messageFormats/message.cpp
               #python/messageFormats/messages.cpp
               #python/messageFormats/dataPacket.cpp
               python/logging.cpp
               #python/proxyBroadcasts/dataPacketSubscriber.cpp
               #python/proxyBroadcasts/dataPacketSubscriberOptions.cpp
               #python/proxyBroadcasts/heartbeat.cpp
               #python/services/command.cpp
               python/messaging.cpp
               python/messageFormats.cpp
               #python/messaging/publisherSubscriber/publisher.cpp
               #python/messaging/publisherSubscriber/subscriberOptions.cpp
               #python/messaging/publisherSubscriber/subscriber.cpp
               )
   set_target_properties(umps_python PROPERTIES
                         CXX_STANDARD 20
                         CXX_STANDARD_REQUIRED YES 
                         CXX_EXTENSIONS NO)
   target_include_directories(umps_python
                              PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/python/include>
                                      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
   target_link_libraries(umps_python
                         PRIVATE pybind11::pybind11 pybind11::lto umps)

   add_library(umpspy MODULE python/umpspy.cpp)
   set_target_properties(umpspy PROPERTIES
                         PREFIX ""
                         CXX_STANDARD 20
                         CXX_STANDARD_REQUIRED YES 
                         CXX_EXTENSIONS NO)
   target_include_directories(umpspy
                              PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/python/include>
                                      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
   target_link_libraries(umpspy
                         PRIVATE pybind11::module umps_python umps)
endif()

##########################################################################################
#                                         Unit Tests                                     #
##########################################################################################
file(COPY ${CMAKE_SOURCE_DIR}/testing/data DESTINATION .)
set(TEST_SRC
    testing/main.cpp
    testing/messageFormats/messages.cpp
    testing/messageFormats/heartbeat.cpp
    testing/messageFormats/failure.cpp
    testing/messageFormats/text.cpp
    testing/messageFormats/heartbeat.cpp
    testing/broadcasts/proxyOptions.cpp
    testing/broadcasts/heartbeat.cpp
    testing/services/proxyOptions.cpp
    testing/services/command.cpp
    testing/services/connectionInformation.cpp
    #testing/services/moduleRegistry.cpp
    testing/messaging/authentication.cpp
    testing/messaging/options.cpp
    )
#if (${BUILD_EW})
#   set(TEST_SRC ${TEST_SRC} testing/messageFormats/earthworm.cpp)
#endif()
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
    testing/communication/remoteCommand.cpp
    testing/communication/pubsub.cpp
    testing/communication/authentication.cpp
    #testing/communication/requestRouter.cpp
    testing/communication/xpubxsub.cpp
    testing/communication/routerDealer.cpp
)
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

if (${WRAP_PYTHON})
   file(COPY ${CMAKE_SOURCE_DIR}/python/unit_test.py DESTINATION .)
#   add_test(NAME python_tests
#            COMMAND ${PYTHON_EXECUTABLE} -m pytest)
endif()

##########################################################################################
#                                        Examples                                        #
##########################################################################################
if (${BUILD_EXAMPLES})
   add_executable(pubSubExample
                  examples/pubSub/main.cpp
                  examples/pubSub/publisher.cpp
                  examples/pubSub/subscriber.cpp)
   set_target_properties(pubSubExample PROPERTIES
                         CXX_STANDARD 20
                         CXX_STANDARD_REQUIRED YES 
                         CXX_EXTENSIONS NO
                         EXCLUDE_FROM_ALL TRUE)
   target_link_libraries(pubSubExample PRIVATE umps)
   target_include_directories(pubSubExample
                              PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

   add_executable(xPubXSubExample
                  examples/xPubXSub/main.cpp
                  examples/xPubXSub/proxy.cpp
                  examples/xPubXSub/publisher.cpp
                  examples/xPubXSub/subscriber.cpp)
   set_target_properties(xPubXSubExample PROPERTIES
                         CXX_STANDARD 20
                         CXX_STANDARD_REQUIRED YES 
                         CXX_EXTENSIONS NO
                         EXCLUDE_FROM_ALL TRUE)
   target_link_libraries(xPubXSubExample PRIVATE umps)
   target_include_directories(xPubXSubExample
                              PRIVATE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>)
endif()


##########################################################################################
#                                      Installation                                      #
##########################################################################################
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)
# create config file
#configure_package_config_file(
    #${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    #"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    #INSTALL_DESTINATION cmake
    #"${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
#)
set(EXECUTABLES uOperator uKeyPair uUserTable uLocalCommand uRemoteCommand)
if (WRAP_PYTHON)
   install(TARGETS umps umpspy umps_python uLocalCommand ${EXECUTABLES}
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else()
   install(TARGETS umps ${EXECUTABLES}
           RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
           LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
           ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
           PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/umps
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES
        #"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        COMPONENT ${PROJECT_NAME})
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
