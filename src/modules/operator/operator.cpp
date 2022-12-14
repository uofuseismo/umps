#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <set>
#include <thread>
#include <mutex>
#ifndef NDEBUG
#include <cassert>
#endif
#include <map>
#include <set>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include "umps/authentication/grasslands.hpp"
#include "umps/authentication/sqlite3Authenticator.hpp"
#include "umps/authentication/service.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/messageFormats/failure.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/service.hpp"
#include "umps/services/command/serviceOptions.hpp"
#include "umps/services/connectionInformation/serviceOptions.hpp"
#include "umps/services/connectionInformation/service.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/modules/process.hpp"
#include "umps/modules/processManager.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/proxyBroadcasts/proxy.hpp"
#include "umps/proxyBroadcasts/proxyOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisher.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcess.hpp"
#include "umps/proxyServices/proxy.hpp"
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/proxyServices/command/proxy.hpp"
#include "umps/proxyServices/command/proxyOptions.hpp"
#include "umps/logging/dailyFile.hpp"

#define MODULE_NAME "uOperator"

namespace UAuth = UMPS::Authentication;
namespace UCI = UMPS::Services::ConnectionInformation;

/// @result Gets the command line input options as a string.
[[nodiscard]] std::string getInputOptions() noexcept
{
    std::string commands;
    commands = "Commands:\n";
    commands = commands + "   quit   Exits the program.\n";
    commands = commands + "   help   Displays this message.\n";
    commands = commands + "   list   Lists the running services.\n";
    return commands;
}

/// @resultGets the input line.
[[nodiscard]] std::string getInputLine() noexcept
{
    return std::string{MODULE_NAME} + "$";
}

struct ProgramOptions
{
    std::vector<UMPS::ProxyServices::ProxyOptions> mProxyServiceOptions;
    std::vector<UMPS::ProxyBroadcasts::ProxyOptions> mProxyBroadcastOptions;
    std::vector<std::pair<int, bool>> mAvailablePorts;
    UCI::ServiceOptions mConnectionInformationOptions;
    UMPS::ProxyServices::Command::ProxyOptions mModuleRegistryOptions;
    //UMPS::Services::ModuleRegistry::ServiceOptions mModuleRegistryOptions;
    UAuth::ZAPOptions mZAPOptions;
    std::string mLogDirectory = "./logs";
    std::string mTablesDirectory = std::string(std::getenv("HOME"))
                                 + "/.local/share/UMPS/tables/";
    std::string mUserTable = mTablesDirectory + "user.sqlite3";
    std::string mBlacklistTable = mTablesDirectory + "blacklist.sqlite3";
    std::string mWhiteListTable = mTablesDirectory + "whitelist.sqlite3";
    std::string mAddress;
    UMPS::Logging::Level mVerbosity = UMPS::Logging::Level::Info;
};

struct Modules
{
    std::map<std::string, std::unique_ptr<UMPS::Services::IService>>
        mServices;
    std::map<std::string, std::unique_ptr<UMPS::ProxyBroadcasts::Proxy>>
        mProxyBroadcasts;
    std::map<std::string, std::unique_ptr<UMPS::ProxyServices::Proxy>>
        mProxyServices;
    std::unique_ptr<UCI::Service> mConnectionInformation;
    std::unique_ptr<UMPS::ProxyServices::Command::Proxy> mModuleRegistry;
};

ProgramOptions parseIniFile(const std::string &iniFile);
std::string parseCommandLineOptions(int argc, char *argv[]);

void printService(const UMPS::Services::IService &service)
{
    try
    {
        auto details = service.getConnectionDetails();
        auto socketInfo = details.getRouterSocketDetails();
        std::cout << "Service: " << service.getName()
                  << " available at: "
                  << service.getRequestAddress() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void printService(const UMPS::ProxyServices::Command::Proxy &proxy)
{
    try
    {
        auto details = proxy.getConnectionDetails();
        auto socketInfo = details.getProxySocketDetails();
        std::cout << "ProxyService: " << proxy.getName()
                  << " frontend available at: "
                  << socketInfo.getFrontendAddress()
                  << " and backend available at: "
                  << socketInfo.getBackendAddress() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void printService(const UMPS::ProxyServices::Proxy &proxy)
{
    try
    {
        auto details = proxy.getConnectionDetails();
        auto socketInfo = details.getProxySocketDetails();
        std::cout << "ProxyService: " << proxy.getName()
                  << " frontend available at: "
                  << socketInfo.getFrontendAddress()
                  << " and backend available at: "
                  << socketInfo.getBackendAddress() << std::endl;
    }
    catch (const std::exception &e)
    {   
        std::cerr << e.what() << std::endl;
    }
}

void printBroadcast(const UMPS::ProxyBroadcasts::Proxy &proxy)
{
    try 
    {   
        auto details = proxy.getConnectionDetails();
        auto socketInfo = details.getProxySocketDetails();
        std::cout << "ProxyBroadcast: " << proxy.getName()
                  << " frontend available at: "
                  << socketInfo.getFrontendAddress()
                  << " and backend available at: "
                  << socketInfo.getBackendAddress() << std::endl;
    }   
    catch (const std::exception &e) 
    {   
        std::cerr << e.what() << std::endl;
    }   
}

/*
std::unique_ptr<UMPS::Modules::IProcess>
    createHeartbeatProcess(const std::string &broadcastName = "Heartbeat",
                           std::shared_ptr<UMPS::Messaging::Context> context = nullptr,
                           std::shared_ptr<UMPS::Logging::ILog> logger = nullptr)
{
    //                                                    "Heartbeat",
    //                                                    context, logger);
}
*/

/*
std::string ipResolver(const std::string &serverName)
{
    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::system::error_code error;
    boost::asio::ip::tcp::resolver::results_type results
        = resolver.resolve(serverName, "", error);
    std::string myIPAddress;
    for (const auto &endPoint : results)
    {
        std::string address = endPoint.endpoint().address().to_string();
        if (!address.empty())
        {
            myIPAddress = address;
            break; // Take the first one
        }
    }
    return myIPAddress;
}
*/

class Operator : public UMPS::Modules::IProcess
{
public:
    Operator() = default;
    /// Destructor
    ~Operator()
    {
        stop();
    }
    /// @result The module name
    [[nodiscard]] std::string getName() const noexcept override
    {
        return "Operator";
    }
    /// @result True indicates this should keep running
    [[nodiscard]] bool keepRunning() const
    {   
        std::scoped_lock lock(mMutex);
        return mKeepRunning; 
    }
    /// @brief Toggles this as running or not running
    void setRunning(const bool running)
    {   
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = running;
    }
    /// @brief Stops the process.
    void stop() override
    {   
        setRunning(false);
    //    if (mBroadcastThread.joinable()){mBroadcastThread.join();}
    //    if (mLocalCommand->isRunning()){mLocalCommand->stop();}
    }
    /// Running?
    [[nodiscard]] bool isRunning() const noexcept override
    {
        return keepRunning();
    }
    // Callback for local interaction
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        commandCallback(const std::string &messageType,
                        const void *data,
                        size_t length)
    {
        namespace USC = UMPS::Services::Command;
        mLogger->debug("Command request received");
        USC::AvailableCommandsRequest availableCommandsRequest;
        USC::CommandRequest commandRequest;
        if (messageType == availableCommandsRequest.getMessageType())
        {
            USC::AvailableCommandsResponse response;
            response.setCommands(getInputOptions());
            try
            {
                availableCommandsRequest.fromMessage(
                    static_cast<const char *> (data), length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed to unpack commands request");
            }
            return response.clone();
        }
        UMPS::MessageFormats::Failure failureResponse;
        failureResponse.setDetails("Unhandled message type: " + messageType);
        return failureResponse.clone();
    } 
    mutable std::mutex mMutex;
    std::unique_ptr<UMPS::Services::Command::Service>
         mLocalCommand{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    bool mKeepRunning{true};
    bool mInitialized{false};
};

///-------------------------------------------------------------------------///
///                                 Main Program                            ///
///-------------------------------------------------------------------------///
int main(int argc, char *argv[])
{
    // Get the ini file from the command line
    std::string iniFile;
    try
    {
        iniFile = parseCommandLineOptions(argc, argv);
        if (iniFile.empty()){return EXIT_SUCCESS;}
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Parse the ini file options 
    std::cout << "Reading initialization file: " << iniFile << std::endl;
    ProgramOptions options;
    try
    {
        options = parseIniFile(iniFile);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Create the authenticator.  Both authenticators will typically say a
    // connection has been established and not much more.
    std::cout << "Initializing connectionInformation service..." << std::endl;
    constexpr int hour = 0;
    constexpr int minute = 0;
    auto authenticatorLogFileName = options.mLogDirectory
                                  + "/" + "authenticator.log";
    UMPS::Logging::DailyFile authenticationLogger;
    authenticationLogger.initialize("Authenticator",
                                    authenticatorLogFileName,
                                    UMPS::Logging::Level::Info, // Always log
                                    hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> authenticationLoggerPtr 
        = std::make_shared<UMPS::Logging::DailyFile> (authenticationLogger);
    std::shared_ptr<UAuth::IAuthenticator> authenticator;
    std::shared_ptr<UAuth::IAuthenticator> adminAuthenticator{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> readOnlyAuthenticator{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> readWriteAuthenticator{nullptr};
    if (options.mZAPOptions.getSecurityLevel() ==
        UAuth::SecurityLevel::Grasslands)
    {
        std::cout << "Creating grasslands authenticator..." << std::endl;
        authenticator
            = std::make_shared<UAuth::Grasslands> (authenticationLoggerPtr);
        adminAuthenticator
            = std::make_shared<UAuth::Grasslands> (authenticationLoggerPtr);
        readOnlyAuthenticator
            = std::make_shared<UAuth::Grasslands> (authenticationLoggerPtr);
        readWriteAuthenticator
            = std::make_shared<UAuth::Grasslands> (authenticationLoggerPtr);
    }
    else
    {
        std::cout << "Creating SQLite3 authenticator..." << std::endl;
        auto sqlite3
            = std::make_shared<UAuth::SQLite3Authenticator>
              (authenticationLoggerPtr, UAuth::UserPrivileges::ReadOnly);
        try
        {
            constexpr bool createIfDoesNotExist = false;
            sqlite3->openUsersTable(options.mUserTable, createIfDoesNotExist);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to open users table.  Failed with: "
                      << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        authenticator = sqlite3; 

        auto admin
            = std::make_shared<UAuth::SQLite3Authenticator>
                     (authenticationLoggerPtr,
                      UAuth::UserPrivileges::Administrator);
        auto readOnly
            = std::make_shared<UAuth::SQLite3Authenticator>
                     (authenticationLoggerPtr, UAuth::UserPrivileges::ReadOnly);
        readOnly->openUsersTable(options.mUserTable, false);
        auto readWrite
            = std::make_shared<UAuth::SQLite3Authenticator>
                    (authenticationLoggerPtr, UAuth::UserPrivileges::ReadWrite);
        readWrite->openUsersTable(options.mUserTable, false);
        adminAuthenticator = admin;
        readOnlyAuthenticator = readOnly;
        readWriteAuthenticator = readWrite;  
    }
    // Initialize the main connection information service
    Modules modules;
    auto connectionInformationLogFileName = options.mLogDirectory + "/"
                                          + "connectionInformation.log";
    UMPS::Logging::DailyFile connectionInformationLogger;
    // Enforce at least info-level logging.  
    auto connectionInformationVerbosity = options.mVerbosity;
    if (connectionInformationVerbosity < UMPS::Logging::Level::Info)
    {
        connectionInformationVerbosity = UMPS::Logging::Level::Info;
    }
    connectionInformationLogger.initialize("ConnectionInformation",
                                           connectionInformationLogFileName,
                                           connectionInformationVerbosity,
                                           hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> connectionInformationLoggerPtr
        = std::make_shared<UMPS::Logging::DailyFile>
          (connectionInformationLogger);
    auto connectionInformation
        = std::make_unique<UCI::Service>
          (connectionInformationLoggerPtr, authenticator);
    connectionInformation->initialize(options.mConnectionInformationOptions);
    modules.mConnectionInformation = std::move(connectionInformation);
    // Initialize the module registry service
    auto moduleRegistryLogFileName = options.mLogDirectory + "/" 
                                   + "moduleRegistry.log";
    UMPS::Logging::DailyFile moduleRegistryLogger;
    moduleRegistryLogger.initialize("ModuleRegistry",
                                    moduleRegistryLogFileName,
                                    UMPS::Logging::Level::Info,
                                    hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> moduleRegistryLoggerPtr
        = std::make_shared<UMPS::Logging::DailyFile> (moduleRegistryLogger);
    auto moduleRegistry
        = std::make_unique<UMPS::ProxyServices::Command::Proxy>
          (moduleRegistryLoggerPtr, adminAuthenticator, readOnlyAuthenticator);
    auto serviceKey = "ProxyServices::" + moduleRegistry->getName();
    moduleRegistry->initialize(options.mModuleRegistryOptions);
    auto moduleRegistryConnectionDetails
        = moduleRegistry->getConnectionDetails();
    modules.mConnectionInformation->addConnection(
        moduleRegistryConnectionDetails);
    moduleRegistry->start();
    // Start the proxy broadcasts
    for (const auto &proxyOptions : options.mProxyBroadcastOptions)
    {
        auto moduleName = proxyOptions.getName();
        std::cout << "Starting " << moduleName
                  << " proxy broadcast" << std::endl;
        auto logFileName = options.mLogDirectory + "/" + moduleName + ".log";
        UMPS::Logging::DailyFile logger; 
        logger.initialize(moduleName, logFileName,
                          options.mVerbosity, hour, minute);
        std::shared_ptr<UMPS::Logging::ILog> loggerPtr
            = std::make_shared<UMPS::Logging::DailyFile> (logger);
        auto proxyBroadcast
            = std::make_unique<UMPS::ProxyBroadcasts::Proxy>
              (loggerPtr, readWriteAuthenticator, readOnlyAuthenticator);
        proxyBroadcast->initialize(proxyOptions); 
        auto broadcastKey = "ProxyBroadcasts::" + moduleName;
        modules.mProxyBroadcasts.insert(std::pair(broadcastKey,
                                                  std::move(proxyBroadcast)));
        try
        {
            modules.mProxyBroadcasts[broadcastKey]->start();
            modules.mConnectionInformation->addConnection(
                *modules.mProxyBroadcasts[broadcastKey]);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    // Start the proxy services
    for (const auto &proxyOptions : options.mProxyServiceOptions)
    {
        auto moduleName = proxyOptions.getName();
        std::cout << "Starting " << moduleName
                  << " proxy service" << std::endl;
        auto logFileName = options.mLogDirectory + "/" + moduleName + ".log";
        UMPS::Logging::DailyFile logger;
        logger.initialize(moduleName, logFileName,
                          options.mVerbosity, hour, minute);
        std::shared_ptr<UMPS::Logging::ILog> loggerPtr
            = std::make_shared<UMPS::Logging::DailyFile> (logger);
        auto proxyService
            = std::make_unique<UMPS::ProxyServices::Proxy>
              (loggerPtr, authenticator);
        proxyService->initialize(proxyOptions);
        auto serviceKey = "ProxyServices::" + moduleName;
        modules.mProxyServices.insert(std::pair(serviceKey,
                                                std::move(proxyService)));
        try
        {
            modules.mProxyServices[serviceKey]->start();
            modules.mConnectionInformation->addConnection(
                *modules.mProxyServices[serviceKey]);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
    // Start the services

    // Start the connection service information 
    std::cout << "Starting connection information service..." << std::endl;
    try
    {
        modules.mConnectionInformation->start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to initialize connection information service"
                  << std::endl;
        return EXIT_FAILURE;
    }
    // Create my processes
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Slow joiner
    UMPS::Modules::ProcessManager
        processManager(connectionInformationLoggerPtr);
    try
    {
        auto processContext = std::make_shared<UMPS::Messaging::Context> (1);
        /*
        const std::string operatorSection{"uOperator"};
        UCI::RequestorOptions requestorOptions;
        auto operatorAddress = options.mConnectionInformationOptions
                                      .getClientAccessAddress();
        requestorOptions.setAddress(operatorAddress);
        requestorOptions.setZAPOptions(options.mZAPOptions);
        auto uOperator
            = std::make_shared<UCI::Requestor> (processContext,
                                                connectionInformationLoggerPtr);
        uOperator->initialize(requestorOptions);
        */

        std::string heartbeatAddress;
        for (const auto &broadcast : modules.mProxyBroadcasts)
        {
            auto details = broadcast.second->getConnectionDetails();
            auto socketInfo = details.getProxySocketDetails();
            if (broadcast.second->getName() == "Heartbeat")
            {
                heartbeatAddress = socketInfo.getFrontendAddress();
            }
        }
        namespace UHB = UMPS::ProxyBroadcasts::Heartbeat;
        UHB::PublisherOptions heartbeatPublisherOptions;
        heartbeatPublisherOptions.setAddress(heartbeatAddress);
        heartbeatPublisherOptions.setZAPOptions(options.mZAPOptions);
        auto heartbeatPublisher
            = std::make_unique<UMPS::ProxyBroadcasts::Heartbeat::Publisher>
              (processContext, connectionInformationLoggerPtr);
        heartbeatPublisher->initialize(heartbeatPublisherOptions);

        UHB::PublisherProcessOptions heartbeatPublisherProcessOptions; 
        auto heartbeatProcess
            = std::make_unique<UMPS::ProxyBroadcasts::Heartbeat::PublisherProcess>
              (connectionInformationLoggerPtr);
        heartbeatProcess->initialize(heartbeatPublisherProcessOptions,
                                     std::move(heartbeatPublisher));
        processManager.insert(std::move(heartbeatProcess));

        // Lastly, start them up
        processManager.start(); 
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to create processes: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    // Main program loop
    while (true)
    {
        std::cout << getInputLine();// << std::endl;
        std::string command;
        std::cin >> command;
        if (command == "help")
        {
            std::cout << getInputOptions() << std::endl;
        }
        else if (command == "quit")
        {
            std::cout << "Quit command received" << std::endl;
            break;
        } 
        else if (command == "list")
        {
            std::cout << "Connection Information:" << std::endl;
            printService(*modules.mConnectionInformation);
            std::cout << std::endl;

            if (!modules.mServices.empty())
            {
                std::cout << "Services:" << std::endl;
                for (const auto &module : modules.mServices)
                {
                    printService(*module.second);
                }
                std::cout << std::endl;
            }

            if (!modules.mProxyBroadcasts.empty())
            {
                std::cout << "ProxyBroadcasts:" << std::endl;
                for (const auto &broadcast : modules.mProxyBroadcasts)
                {
                    printBroadcast(*broadcast.second);
                }
                std::cout << std::endl;
            }

            if (!modules.mProxyServices.empty() || moduleRegistry->isRunning())
            {
                std::cout << "ProxyServices:" << std::endl;
                if (moduleRegistry->isRunning())
                {
                    printService(*moduleRegistry);
                }
                for (const auto &service : modules.mProxyServices)
                {
                    printService(*service.second);
                }
                std::cout << std::endl;
            }
        }
        else
        {
            std::cout << "Unknown command: " << command;
        }
//        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    processManager.stop();
    // Shut down the modules under remote management
    std::cout << "Stopping the module registry..." << std::endl;
    moduleRegistry->stop(); 
    // Shut down the services
    //std::cout << "Stopping the authenticator..." << std::endl;
    //authenticatorService.stop();
    std::cout << "Stopping the proxy broadcasts..." << std::endl;
    for (auto &broadcast : modules.mProxyBroadcasts)
    {
        modules.mConnectionInformation->removeConnection(
            broadcast.second->getName());
        broadcast.second->stop();
    } 
    std::cout << "Stopping the proxy services..." << std::endl;
    for (auto &service : modules.mProxyServices)
    {
        modules.mConnectionInformation->removeConnection(
            service.second->getName());
        service.second->stop();
    }
    std::cout << "Stopping the services..." << std::endl;
    for (auto &service : modules.mServices)
    {
        modules.mConnectionInformation->removeConnection(
            service.second->getName());
        service.second->stop();
    }
    //if (modules.mDataPacketBroadcast){modules.mDataPacketBroadcast->stop();}
    //if (modules.mHeartbeatBroadcast){modules.mHeartbeatBroadcast->stop();}
    std::cout << "Stopping the connection information service..." << std::endl;
    modules.mConnectionInformation->stop();
    // Join the threads
/*
    for (auto &thread : threads)
    {
        if (thread.joinable()){thread.join();}
    }
*/
    return EXIT_SUCCESS;
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
/// Read the program options from the command line
std::string parseCommandLineOptions(int argc, char *argv[])
{
    std::string iniFile;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("ini",  boost::program_options::value<std::string> (),
                 "Defines the initialization file for this module");
    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return iniFile;
    }
    if (vm.count("ini"))
    {
        iniFile = vm["ini"].as<std::string>();
        if (!std::filesystem::exists(iniFile))
        {
            throw std::runtime_error("Initialization file: " + iniFile
                                   + " does not exist");
        }
    }
    else
    {
        throw std::runtime_error("Initialization file was not set");
    }
    return iniFile;
}

/// @brief Utility to make the next available tcp address given a list of
///        available ports.
/// @param[in] proposedAddress     The proposed address to assign.
/// @param[in,out] availablePorts  The available ports for TCP connections.
/// @param[in] rootAddress         The root address for tcp connections e.g.,
///                                127.0.0.1
/// @param[in] usedAddresses       A list of addresses already in use.
/// @param[in] connectionType      The connection type - e.g., tcp://, ipc://,
///                                or inproc://
/// @result The next available address.
std::string makeNextAvailableAddress(
    const std::string &proposedAddress,
    std::vector<std::pair<int, bool>> *availablePorts,
    const std::string &rootAddress,
    const std::set<std::string> &usedAddresses,
    const std::string &connectionType = "tcp://")
{
    // If the proposed address isn't in use then we're done
    if (!proposedAddress.empty())
    {
        if (!usedAddresses.contains(proposedAddress))
        {
            return proposedAddress;
        }
    }
    // For TCP connections we append port numbers.  We simply need to check
    // these ports are still available.
    std::string address;
    if (connectionType.find("tcp://") == 0)
    {
        for (auto &availablePort : *availablePorts)
        {
            auto port = availablePort.first;
            auto isAvailable = availablePort.second;
            if (isAvailable)
            {
                address = connectionType + rootAddress
                        + ":" + std::to_string(port);
                if (usedAddresses.contains(address))
                {
                    continue;
                }
                //usedAddresses->insert(address); 
                availablePort = std::pair(port, false);
                break;
            }
        }
        if (address.empty())
        {
            throw std::runtime_error("All ports are exhausted");
        }
    }
    else
    {
        address = connectionType + rootAddress;
        if (usedAddresses.contains(address))
        {
            throw std::runtime_error("Proposed address already exists");
        }
    }
    return address;
}

/// @brief Given a proposed this address, this function checks if the address
///        is in use, and if yes, returns the next valid address.
/// @param[in] proposedAddressIn  The proposed address.  This can be blank.
/// @param[in,out] usedAddresses  The addresses currently in use.  On exit,
///                               the new proposed address will be added to
///                               this.
/// @param[in,out] options        The available ports are updated in the case
///                               of TCP connections.
/// @param[in] connectionType     The connection type - e.g., tcp://.
/// @result An available address to bind/connect to.
[[nodiscard]]
std::string makeNewAddress(const std::string &proposedAddressIn,
                           std::set<std::string> *usedAddresses,
                           ProgramOptions *options,
                           const std::string &connectionType)
{
    auto proposedAddress = proposedAddressIn;
    auto newProposedAddress
        = makeNextAvailableAddress(proposedAddress,
                                   &options->mAvailablePorts,
                                   options->mAddress,
                                   *usedAddresses,
                                   connectionType);
    if (newProposedAddress != proposedAddress)
    {
        proposedAddress = newProposedAddress;
    }
    usedAddresses->insert(proposedAddress);
    return proposedAddress;
}

/// @brief Convenience function for making a frontend/backend address pair.
/// @sa makeNewAddress
std::pair<std::string, std::string>
    makeNewFrontendBackendAddress(const std::string &proposedFrontendAddress,
                                  const std::string &proposedBackendAddress,
                                  std::set<std::string> *usedAddresses,
                                  ProgramOptions *options,
                                  const std::string &connectionType)
{
    auto frontendAddress = makeNewAddress(proposedFrontendAddress,
                                          usedAddresses,
                                          options,
                                          connectionType);
    auto backendAddress = makeNewAddress(proposedBackendAddress,
                                          usedAddresses,
                                          options,
                                          connectionType);
    return std::pair {frontendAddress, backendAddress};
}

/// @brief Convenience function to add a proxy broadcast to the program options.
/// @param[in,out] options        The options with available ports and
///                               addresses.  This will be updated with a
///                               set of proxy broadcast options for this
///                               broadcast.
/// @param[in,out] usedAddresses  A list of used addresses.  This will be
///                               updated with a new frontend and backend
///                               address.
/// @param[in] iniFile         The iniitializion file to parse.
/// @param[in] proxyBroadcast  The name of the proxy broadcast.
/// @param[in] connectionType  The connection type - e.g., "tcp://"
void addProxyBroadcast(ProgramOptions *options,
                       std::set<std::string> *usedAddresses,
                       const std::string &iniFile,
                       const std::string &proxyBroadcast,
                       const std::string &connectionType)
{
    UMPS::ProxyBroadcasts::ProxyOptions proxyOptions;
    proxyOptions.parseInitializationFile(iniFile, proxyBroadcast);
    proxyOptions.setZAPOptions(options->mZAPOptions);
    // Lift candidate addresses from proxy options
    std::string proposedFrontendAddress;
    if (proxyOptions.getProxyOptions().haveFrontendAddress())
    {
        proposedFrontendAddress
            = proxyOptions.getProxyOptions().getFrontendAddress();
    }
    std::string proposedBackendAddress;
    if (proxyOptions.getProxyOptions().haveBackendAddress())
    {
        proposedBackendAddress
            = proxyOptions.getProxyOptions().getBackendAddress();
    }
    // Make and insert new addresses into the used addresses
    auto [frontendAddress, backendAddress]
         = makeNewFrontendBackendAddress(proposedFrontendAddress,
                                         proposedBackendAddress,
                                         usedAddresses,
                                         options,
                                         connectionType);
    // Save these addresses 
    proxyOptions.setFrontendAddress(frontendAddress);
    proxyOptions.setBackendAddress(backendAddress);
    options->mProxyBroadcastOptions.push_back(proxyOptions);
}

/// @brief Convenience function to add a proxy service to the program options.
/// @param[in,out] options        The options with available ports and
///                               addresses.  This will be updated with a
///                               set of proxy service options for this
///                               service.
/// @param[in,out] usedAddresses  A list of used addresses.  This will be
///                               updated with a new frontend and backend
///                               address.
/// @param[in] iniFile         The iniitializion file to parse.
/// @param[in] proxyService    The name of the proxy service.
/// @param[in] connectionType  The connection type - e.g., "tcp://"
void addProxyService(ProgramOptions *options,
                     std::set<std::string> *usedAddresses,
                     const std::string &iniFile,
                     const std::string &proxyService,
                     const std::string &connectionType)
{
    UMPS::ProxyServices::ProxyOptions proxyOptions;
    proxyOptions.parseInitializationFile(iniFile, proxyService);
    proxyOptions.setZAPOptions(options->mZAPOptions);
    // Lift candidate addresses from proxy options
    std::string proposedFrontendAddress;
    if (proxyOptions.getProxyOptions().haveFrontendAddress())
    {
        proposedFrontendAddress
            = proxyOptions.getProxyOptions().getFrontendAddress();
    }
    std::string proposedBackendAddress;
    if (proxyOptions.getProxyOptions().haveBackendAddress())
    {
        proposedBackendAddress
            = proxyOptions.getProxyOptions().getBackendAddress();
    }
    // Make and insert new addresses into the used addresses
    auto [frontendAddress, backendAddress]
         = makeNewFrontendBackendAddress(proposedFrontendAddress,
                                         proposedBackendAddress,
                                         usedAddresses,
                                         options,
                                         connectionType);
    // Save these addresses 
    proxyOptions.setFrontendAddress(frontendAddress);
    proxyOptions.setBackendAddress(backendAddress);
    options->mProxyServiceOptions.push_back(proxyOptions);
/*
    std::string address;
    if (proxyOptions.getProxyOptions().haveFrontendAddress())
    {
        address = proxyOptions.getProxyOptions().getFrontendAddress();
    }
    auto newAddress = makeNextAvailableAddress(address,
                                               &options->mAvailablePorts,
                                               options->mAddress,
                                               *usedAddresses,
                                               connectionType);
    if (newAddress != address)
    {
        proxyOptions.setFrontendAddress(newAddress);
    }
    usedAddresses->insert(newAddress);
    // Backend address 
    if (proxyOptions.getProxyOptions().haveBackendAddress())
    {
        address = proxyOptions.getProxyOptions().getBackendAddress();
    }
    newAddress = makeNextAvailableAddress(address,
                                          &options->mAvailablePorts,
                                          options->mAddress,
                                          *usedAddresses,
                                          connectionType);
    if (newAddress != address)
    {
        proxyOptions.setBackendAddress(newAddress);
    }
    options->mProxyServiceOptions.push_back(proxyOptions);
    usedAddresses->insert(newAddress);
*/
}

/// @brief Creates a list of properties corresponding to the property name.
/// @param[in] propertyTree   The property tree.
/// @param[in] propertyName   The property name to search for subentities,
///                           e.g., ProxyBroadcasts.
/// @result A set of properties with the property name contained.  For example,
///         [ProxyBroadcasts:Heartbeat]
///         [ProxyBroadcasts:Data]
///         would return {ProxyBroadcasts:Heartbeat, ProxyBroadcasts:Data}
std::set<std::string>
makePropertyList(const boost::property_tree::ptree &propertyTree,
                 const std::string &propertyName)
{
    std::set<std::string> result;
    for (const auto &p : propertyTree)
    {
        if (p.first.find(propertyName) != std::string::npos)
        {
            auto item = p.first;
            if (result.contains(item))
            {
                std::cerr << "Warning!  " << propertyName << ": " << item
                              << " already exists; skipping..."
                              << std::endl;
                break;
            }
            result.insert(item);
        }
    }
    return result;
}

///--------------------------------------------------------------------------///
///                          Parse the ini file                              ///
///--------------------------------------------------------------------------///
ProgramOptions parseIniFile(const std::string &iniFile)
{
    std::set<std::string> requiredProxyServices{};
    std::set<std::string> requiredProxyBroadcasts{ "Heartbeat" };
    std::set<std::string> requiredservices{ "uOperator" };
    std::set<std::string> reservedNames{"uOperator", "ModuleRegistry"};
    const std::string connectionType = "tcp://";
    ProgramOptions options;
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }
    std::set<std::string> usedAddresses;
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Verbosity
    options.mVerbosity
        = static_cast<UMPS::Logging::Level>
          (propertyTree.get<int> ("uOperator.verbosity",
                                  static_cast<int> (options.mVerbosity)));
    // Need an IP address
    options.mAddress = propertyTree.get<std::string> ("uOperator.address");
    if (options.mAddress.empty())
    {
        options.mAddress = propertyTree.get<std::string> ("uOperator.ipAddress");
        if (options.mAddress.empty())
        {
            throw std::runtime_error("uOperator.address not not defined");
        }
    }
    auto portStart = propertyTree.get<int> ("uOperator.openPortBlockStart",
                                            8000);
    auto portEnd   = propertyTree.get<int> ("uOperator.openPortBlockEnd",
                                            8899);
    if (portStart < 6)
    {
        throw std::runtime_error("uOperator.openPortBlockStart = "
                               + std::to_string(portStart)
                               + " must be at least 6");
    }
    if (portEnd < portStart)
    {
        throw std::runtime_error("uOperator.openPortBlockEnd = "
                       + std::to_string(portEnd)
                       + " must be greater than uOperator.openPortBlockStart = "
                       + std::to_string(portStart));
    }
    int nPorts = portEnd - portStart + 1;
    options.mAvailablePorts.reserve(nPorts);
    for (int port = portStart; port <= portEnd; ++port)
    {
        options.mAvailablePorts.push_back(std::pair(port, true));
    }
    // Parse the general properties
    options.mLogDirectory = propertyTree.get<std::string>
        ("uOperator.logFileDirectory", options.mLogDirectory);
    if (!options.mLogDirectory.empty() &&
        !std::filesystem::exists(options.mLogDirectory))
    {
        std::cout << "Creating log file directory: "
                  << options.mLogDirectory << std::endl;
        if (!std::filesystem::create_directories(options.mLogDirectory))
        {
            throw std::runtime_error("Failed to make log directory");
        }
    }
    // Define ZAP options
    options.mZAPOptions
        = UMPS::Modules::Operator::readZAPServerOptions(iniFile, "uOperator");
    auto securityLevel = options.mZAPOptions.getSecurityLevel();
    // Get sqlite3 authentication tables
    if (securityLevel != UAuth::SecurityLevel::Grasslands)
    {
        options.mTablesDirectory
            = propertyTree.get<std::string> ("uOperator.tablesTableDirectory",
                                             options.mTablesDirectory);
        options.mUserTable
            = propertyTree.get<std::string> ("uOperator.userTable",
                                             options.mUserTable);
        if (!std::filesystem::exists(options.mUserTable) &&
            (securityLevel == UAuth::SecurityLevel::Woodhouse ||
            securityLevel == UAuth::SecurityLevel::Stonehouse))
        {
            throw std::invalid_argument("User table: "
                                      + options.mUserTable + " does not exist");
        }
        options.mBlacklistTable 
            = propertyTree.get<std::string> ("uOperator.blackListTable",
                                             options.mBlacklistTable);
        options.mWhiteListTable
            = propertyTree.get<std::string> ("uOperator.whiteListTable",
                                             options.mWhiteListTable);
    }
    // Set the ZAP information for the connection information service
    options.mConnectionInformationOptions.setZAPOptions(options.mZAPOptions);
    // First make sure the connection information service is available
    auto address = connectionType + options.mAddress
                 + ":" + std::to_string(options.mAvailablePorts.at(0).first);
    options.mConnectionInformationOptions.setClientAccessAddress(address);
    options.mAvailablePorts.at(0).second = false;
    // Also, need the module registry
    {
        UMPS::ProxyServices::Command::ProxyOptions moduleRegistryOptions;
        moduleRegistryOptions.parseInitializationFile(iniFile,
                                                      "ModuleRegistry");
        moduleRegistryOptions.setZAPOptions(options.mZAPOptions);
        std::string proposedFrontendAddress;
        if (moduleRegistryOptions.haveFrontendAddress())
        {
            proposedFrontendAddress
                = moduleRegistryOptions.getFrontendAddress();
        }
        std::string proposedBackendAddress;
        if (moduleRegistryOptions.haveBackendAddress())
        {
            proposedBackendAddress
                = moduleRegistryOptions.getBackendAddress();
        }
        auto [frontendAddress, backendAddress]
             = makeNewFrontendBackendAddress(proposedFrontendAddress,
                                             proposedBackendAddress,
                                             &usedAddresses,
                                             &options,
                                             connectionType);
        moduleRegistryOptions.setFrontendAddress(frontendAddress);
        moduleRegistryOptions.setBackendAddress(backendAddress);
        options.mModuleRegistryOptions = moduleRegistryOptions;
    }
    // Next, search for any essential broadcasts or services.  These addresses
    // need to be claimed first.  Additionally, the user may want them
    // associated with specific addresses so we allow that to happen here.
    auto proxyBroadcasts = makePropertyList(propertyTree, "ProxyBroadcasts"); 
    auto proxyServices = makePropertyList(propertyTree, "ProxyServices");
    for (const auto &proxyBroadcast : proxyBroadcasts)
    {
        auto broadcastName = proxyBroadcast;
        boost::algorithm::replace_all(broadcastName, "ProxyBroadcasts:", "");
        if (reservedNames.contains(broadcastName))
        {
            throw std::runtime_error("Invalid broadcast name: " + broadcastName
                                   + ".  Matches reserved name");
        }
        if (requiredProxyBroadcasts.contains(broadcastName))
        {
            addProxyBroadcast(&options, &usedAddresses,
                              iniFile, proxyBroadcast, connectionType);
        }
    }
    for (const auto &proxyService : proxyServices)
    {
        auto serviceName = proxyService;
        boost::algorithm::replace_all(serviceName, "ProxyServices:", "");
        if (reservedNames.contains(serviceName))
        {
            throw std::runtime_error("Invalid service name: " + serviceName
                                   + ".  Matches reserved name");
        }
        if (requiredProxyServices.contains(serviceName))
        {
            addProxyService(&options, &usedAddresses,
                            iniFile, proxyService, connectionType);
        }
    }
    // Next, the module registry
/*
    address = connectionType + options.mAddress
            + ":" + std::to_string(options.mAvailablePorts.at(1).first);
    options.mModuleRegistryOptions.setZAPOptions(options.mZAPOptions);
    options.mModuleRegistryOptions.setClientAccessAddress(address);
    options.mAvailablePorts.at(1).second = false;
*/
    // Now get the remaining proxy broadcasts.
    for (const auto &proxyBroadcast : proxyBroadcasts)
    {
        auto broadcastName = proxyBroadcast;
        boost::algorithm::replace_all(broadcastName, "ProxyBroadcasts:", ""); 
        if (!requiredProxyBroadcasts.contains(broadcastName))
        {
            addProxyBroadcast(&options, &usedAddresses,
                              iniFile, proxyBroadcast, connectionType);
        }
    }
    // And the remaining proxy services.
    for (const auto &proxyService : proxyServices)
    {
        auto serviceName = proxyService;
        boost::algorithm::replace_all(serviceName, "ProxyServices:", ""); 
        if (!requiredProxyServices.contains(serviceName))
        {
            addProxyService(&options, &usedAddresses,
                            iniFile, proxyService, connectionType);
        }
    }
    return options;
}
