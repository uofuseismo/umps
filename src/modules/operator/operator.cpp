#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <map>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include "umps/messaging/authentication/grasslands.hpp"
#include "umps/messaging/authentication/sqlite3Authenticator.hpp"
#include "umps/services/connectionInformation/parameters.hpp"
#include "umps/services/connectionInformation/service.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/incrementer/service.hpp"
#include "umps/services/incrementer/parameters.hpp"
#include "umps/broadcasts/dataPacket/broadcast.hpp"
#include "umps/broadcasts/dataPacket/parameters.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/spdlog.hpp"

namespace UAuth = UMPS::Messaging::Authentication;

struct ProgramOptions
{
    std::vector<UMPS::Services::Incrementer::Parameters> mIncrementerParameters;
    std::vector<std::pair<int, bool>> mAvailablePorts;
    UMPS::Services::ConnectionInformation::Parameters
        mConnectionInformationParameters;
    UMPS::Broadcasts::DataPacket::Parameters mDataPacketParameters;
    std::string mLogDirectory = "./logs";
    std::string mTablesDirectory = std::string(std::getenv("HOME"))
                                 + "/.local/share/UMPS/tables/";
    std::string mUserTable = mTablesDirectory + "user.sqlite3";
    std::string mBlacklistTable = mTablesDirectory + "blacklist.sqlite3";
    std::string mWhiteListTable = mTablesDirectory + "whitelist.sqlite3";
    std::string mIPAddress;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS;
};

struct Modules
{
    std::vector<UMPS::Services::Incrementer::Service> mIncrementers;
    std::vector<UMPS::Broadcasts::IBroadcast> mBroadcasts;
    UMPS::Broadcasts::DataPacket::Broadcast mDataPacketBroadcast;
    UMPS::Services::ConnectionInformation::Service mConnectionInformation;
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

std::string makeNextAvailableAddress(
    std::vector<std::pair<int, bool>> &availablePorts,
    const std::string &ipAddress,
    const std::string &connectionType = "tcp://")
{
    std::string address;
    for (auto &availablePort : availablePorts)
    {
        auto port = availablePort.first;
        auto isAvailable = availablePort.second;
        if (isAvailable)
        {
            address = connectionType + ipAddress
                    + ":" + std::to_string(port);
            availablePort = std::pair(port, false);
            break;
        }
    }
    if (address.empty())
    {
        throw std::runtime_error("All ports are exhausted");
    }
    return address;
}

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

///-------------------------------------------------------------------------///
///                                 Main Program                            ///
///-------------------------------------------------------------------------///
int main(int argc, char *argv[])
{
    //auto myIPAddress = ipResolver("waldorf.seis.utah.edu");
    //std::cout << myIPAddress << std::endl;
    //return 0;
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
    // Create the authenticator
    const int hour = 0;
    const int minute = 0;
    auto authenticatorLogFileName = options.mLogDirectory
                                  + "/" + "authenticator.log";
    UMPS::Logging::SpdLog authenticationLogger;
    authenticationLogger.initialize("Authenticator",
                                    authenticatorLogFileName,
                                    UMPS::Logging::Level::INFO,
                                    hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> authenticationLoggerPtr 
        = std::make_shared<UMPS::Logging::SpdLog> (authenticationLogger); 
    std::shared_ptr<UAuth::IAuthenticator> authenticator;
    if (options.mSecurityLevel == UAuth::SecurityLevel::GRASSLANDS)
    {
        std::cout << "Creating grasslands authenticator..." << std::endl;
        authenticator
            = std::make_shared<UAuth::Grasslands> (authenticationLoggerPtr);
    }
    else
    {
        std::cout << "Creating SQLite3 authenticator..." << std::endl;
        auto sqlite3 = std::make_shared<UAuth::SQLite3Authenticator>
                       (authenticationLoggerPtr);
//        //sqlite3->initialize( );
//        authenticator = sqlite3; 
    }
    // Initialize the services
    Modules modules;
    auto connectionInformationLogFileName = options.mLogDirectory + "/"
                                          + "connectionInformation.log";
    UMPS::Logging::SpdLog connectionInformationLogger;
    connectionInformationLogger.initialize("ConnectionInformation",
                                           connectionInformationLogFileName,
                                           UMPS::Logging::Level::DEBUG,
                                           hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> connectionInformationLoggerPtr
        = std::make_shared<UMPS::Logging::SpdLog> (connectionInformationLogger);
    UMPS::Services::ConnectionInformation::Service
        connectionInformation(connectionInformationLoggerPtr);
    connectionInformation.initialize(options.mConnectionInformationParameters);
    modules.mConnectionInformation = std::move(connectionInformation);

    //modules.mConnectionInformation.initialize(
     //   options.mConnectionInformationParameters);
    

    modules.mIncrementers.reserve(options.mIncrementerParameters.size());
    for (const auto &parameters : options.mIncrementerParameters)
    {
        auto modulesName = "incrementer_" + parameters.getName();
        auto logFileName = options.mLogDirectory + "/" + modulesName + ".log";
        UMPS::Logging::SpdLog logger;
        logger.initialize(modulesName, logFileName,
                          parameters.getVerbosity(), hour, minute);
        std::shared_ptr<UMPS::Logging::ILog> loggerPtr
           = std::make_shared<UMPS::Logging::SpdLog> (logger);
        UMPS::Services::Incrementer::Service service(loggerPtr);
        try
        {
            service.initialize(parameters);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to initialize incrementer service"
                      << std::endl;
        }
        modules.mIncrementers.push_back(std::move(service));
    }
    // Start the connection information service
    std::vector<std::thread> threads;
    std::cout << "Starting connection information service..." << std::endl;
    try
    {
        std::thread t(&UMPS::Services::ConnectionInformation::Service::start,
                      &modules.mConnectionInformation); 
        threads.push_back(std::move(t));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to initialize connection information service"
                  << std::endl;
    }
    // Now start the services - a thread per service
    std::cout << "Starting incrementer services..." << std::endl;
    for (auto &module : modules.mIncrementers)
    {
        try
        {
            std::thread t(&UMPS::Services::IService::start, //Incrementer::Service::start,
                          &module);
            threads.push_back(std::move(t));
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            continue;
        }
        modules.mConnectionInformation.addConnection(module);
    }
    // And start the broadcasts...
    try
    {
        std::cout << "Data packet service..." << std::endl;
        modules.mDataPacketBroadcast.initialize(options.mDataPacketParameters);
        std::thread t(&UMPS::Broadcasts::IBroadcast::start,
                      &modules.mDataPacketBroadcast);
        threads.push_back(std::move(t));
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
//    std::thread t(&UMPS::Broadcasts::DataPacket::Broadcast::start,
                  
    // Main program loop
    while (true)
    {
        std::cout << std::endl << "uOperator$";
        std::string command;
        std::cin >> command;
        if (command == "help")
        {
            std::cout << "Commands:" << std::endl;
            std::cout << "  quit - exits the program" << std::endl;
            std::cout << "  help - shows this message" << std::endl;
            std::cout << "  list - lists the running services" << std::endl;
        }
        else if (command == "quit")
        {
            std::cout << "Quit command received" << std::endl;
            break;
        } 
        else if (command == "list")
        {
            std::cout << "Connection Information:" << std::endl;
            printService(modules.mConnectionInformation);
            std::cout << std::endl;

            std::cout << "Incrementers:" << std::endl;
            for (const auto &module : modules.mIncrementers)
            {
                printService(module);
            }
            std::cout << std::endl;

            std::cout << "Broadcasts:" << std::endl;
        }
        else
        {
            std::cout << "Unknown command: " << command;
        }
//        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shut down the services
    std::cout << "Stopping incrementer services..." << std::endl;
    for (auto &module : modules.mIncrementers)
    {
        modules.mConnectionInformation.removeConnection(module.getName());
        module.stop();
    }
    modules.mDataPacketBroadcast.stop();
    modules.mConnectionInformation.stop();
    // Join the threads
    for (auto &thread : threads)
    {
        if (thread.joinable()){thread.join();}
    }
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

///--------------------------------------------------------------------------///
/// Parse the ini file
ProgramOptions parseIniFile(const std::string &iniFile)
{
    ProgramOptions options;
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Need an IP address
    options.mIPAddress = propertyTree.get<std::string> ("uOperator.ipAddress");
    if (options.mIPAddress.empty())
    {
        throw std::runtime_error("uOperator.ipAddress not not defined");
    }
    auto portStart = propertyTree.get<int> ("uOperator.openPortBlockStart",
                                            8000);
    auto portEnd   = propertyTree.get<int> ("uOperator.openPortBlockEnd",
                                            8899);
    if (portStart < 3)
    {
        throw std::runtime_error("uOperator.openPortBlockStart = "
                               + std::to_string(portStart)
                               + " must be at least 3");
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
    auto securityLevel
        = propertyTree.get<int> ("uOperator.securityLevel",
                                 static_cast<int> (options.mSecurityLevel));
    if (securityLevel < 0 || securityLevel > 4)
    {
        throw std::invalid_argument("Security level must be in range [0,4]");
    }
    options.mSecurityLevel = static_cast<UAuth::SecurityLevel> (securityLevel);
    // Get sqlite3 authentication tables
    if (options.mSecurityLevel != UAuth::SecurityLevel::GRASSLANDS)
    {
        options.mTablesDirectory
            = propertyTree.get<std::string> ("uOperator.tablesTableDirectory",
                                             options.mTablesDirectory);
        options.mUserTable
            = propertyTree.get<std::string> ("uOperator.userTable",
                                             options.mUserTable);
        if (!std::filesystem::exists(options.mUserTable))
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
    // First make sure the connection information service is available
    auto address = "tcp://" + options.mIPAddress
                 + ":" + std::to_string(options.mAvailablePorts.at(0).first);
    options.mConnectionInformationParameters.setClientAccessAddress(address);
    options.mAvailablePorts.at(0).second = false;
    // Next get all the counters
    std::vector<std::string> counters;
    for (const auto &p : propertyTree)
    {
        if (p.first.find("Counters") != std::string::npos)
        {
            auto lExists = false;
            for (const auto &c : counters)
            {
                if (c == p.first)
                {
                    std::cerr << "Warning!  Counter: " << c
                              << " already exists; skipping..."
                              << std::endl;
                    lExists = true;
                    break;
                }
            }
            if (!lExists){counters.push_back(p.first);}
        }
    }
    // Parse the options for each counter
    for (const auto &counter : counters)
    {
        UMPS::Services::Incrementer::Parameters counterOptions;
        try
        {
            counterOptions.parseInitializationFile(iniFile, counter);
        }
        catch (const std::exception &e) 
        {
           std::cerr << "Failed to read incrementer options for: " << counter
                     << " Failed with:\n" << e.what() << std::endl;
        }
        // Assign an IP address
        if (!counterOptions.haveClientAccessAddress())
        {
            auto counterAddress = makeNextAvailableAddress(
                                      options.mAvailablePorts, 
                                      options.mIPAddress,
                                      "tcp://");
            counterOptions.setClientAccessAddress(counterAddress);
        }
#ifndef NDEBUG
        assert(counterOptions.haveClientAccessAddress());
#endif
        options.mIncrementerParameters.push_back(std::move(counterOptions));
    }
    // Parse the datapacket broadcast options 
    UMPS::Broadcasts::DataPacket::Parameters dataPacketOptions;
    try
    {
        dataPacketOptions.parseInitializationFile(iniFile, "Broadcasts");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to read datapacket broadcast options."
                  << "Failed with:\n" << e.what()  << std::endl;
    }
    if (!dataPacketOptions.haveFrontendAddress())
    {
        auto frontendAddress = makeNextAvailableAddress(
                                    options.mAvailablePorts,
                                    options.mIPAddress,
                                    "tcp://");
        dataPacketOptions.setFrontendAddress(frontendAddress);
    } 
    if (!dataPacketOptions.haveBackendAddress())
    {
        auto backendAddress = makeNextAvailableAddress(
                                    options.mAvailablePorts,
                                    options.mIPAddress,
                                    "tcp://");
        dataPacketOptions.setBackendAddress(backendAddress);
    }
    if (dataPacketOptions.haveFrontendAddress() &&
        dataPacketOptions.haveBackendAddress())
    {
        options.mDataPacketParameters = dataPacketOptions;
    }
 
    return options;
}