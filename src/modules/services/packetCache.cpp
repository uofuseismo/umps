#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/proxyServices/packetCache/service.hpp"
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/proxyServices/packetCache/sensorRequest.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
#include "umps/proxyServices/packetCache/replier.hpp"
#include "umps/proxyServices/packetCache/replierOptions.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriber.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "private/threadSafeQueue.hpp"
#include "private/staticUniquePointerCast.hpp"
#include "private/isEmpty.hpp"

namespace UPacketCache = UMPS::ProxyServices::PacketCache;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;
namespace UAuth = UMPS::Authentication;
namespace UCI = UMPS::Services::ConnectionInformation;

#define DEFAULT_HWM 4096
#define DEFAULT_TIMEOUT std::chrono::milliseconds{10}
#define DEFAULT_MAXPACKETS 200

struct ProgramOptions
{
    UCI::RequestorOptions mConnectionInformationRequestOptions;
    UPacketCache::ReplierOptions mPacketCacheReplyOptions;
    UAuth::ZAPOptions mZAPOptions;
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPacket";
    std::string proxyServiceName;
    std::string mLogFileDirectory = "logs";
    std::chrono::milliseconds dataPacketTimeOut{10};
    UMPS::Logging::Level mVerbosity = UMPS::Logging::Level::INFO;
    int maxPackets = DEFAULT_MAXPACKETS;
    int dataPacketHighWaterMark = static_cast<int> (DEFAULT_HWM);
};

/// Parses the command line options
[[nodiscard]] std::string parseOptions(int argc, char *argv[]);
[[nodiscard]] ProgramOptions parseInitializationFile(const std::string &iniFile);

///-------------------------------------------------------------------------///
///                                 Main Program                            ///
///-------------------------------------------------------------------------///
int main(int argc, char *argv[])
{
    // Get the program options
    std::string iniFile;
    try
    {
        iniFile = parseOptions(argc, argv); 
        if (iniFile.empty()){return EXIT_SUCCESS;}
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Read the configuration file
    ProgramOptions options;
    try
    {
        options = parseInitializationFile(iniFile);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    auto zapOptions = options.mZAPOptions;
    // Create logger
    UMPS::Logging::StdOut logger;
    logger.setLevel(options.mVerbosity); //UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Get the connection details
    logger.debug("Getting available services...");
    std::string dataPacketAddress;
    std::string proxyServiceAddress;
    try
    {
        UCI::Requestor connectionInformation;
        connectionInformation.initialize(
            options.mConnectionInformationRequestOptions);
        dataPacketAddress
            = connectionInformation.getProxyBroadcastBackendDetails(
                 options.dataBroadcastName).getAddress();
        proxyServiceAddress
            = connectionInformation.getProxyServiceBackendDetails(
                 options.proxyServiceName).getAddress();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        logger.error(std::string {e.what()});
        return EXIT_FAILURE;
    }
    // Set the broadcast connection details
    UMPS::ProxyBroadcasts::DataPacket::SubscriberOptions<double>
        dataPacketSubscriberOptions;
    dataPacketSubscriberOptions.setAddress(dataPacketAddress);
    dataPacketSubscriberOptions.setHighWaterMark(
        options.dataPacketHighWaterMark);
    dataPacketSubscriberOptions.setTimeOut(options.dataPacketTimeOut);
    dataPacketSubscriberOptions.setZAPOptions(zapOptions);
    // Set the reply options service
    options.mPacketCacheReplyOptions.setAddress(proxyServiceAddress);
    options.mPacketCacheReplyOptions.setZAPOptions(zapOptions);

    // Initialize the service
    UPacketCache::Service<double> service(loggerPtr);
    try
    {
        logger.debug("Starting the service...");
        service.initialize(options.maxPackets,
                           dataPacketSubscriberOptions,
                           options.mPacketCacheReplyOptions);
        service.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to start service.  Failed with: " 
                  << e.what() << std::endl;
        logger.error("Failed to start service.  Failed with: "
                   + std::string {e.what()});
        return EXIT_FAILURE;
    }
    // Monitor commands from stdin
    while (true)
    {
        std::string command;
        std::cout << "packetCache$";
        std::cin >> command;
        if (command == "quit")
        {
            break;
        }
        else if (command == "cacheSize")
        {
            std::cout << "Total number of packets in packet cache: "
                      << service.getTotalNumberOfPackets() << std::endl;
        }
        else
        {
            std::cout << std::endl;
            if (command != "help")
            {
                std::cout << "Unknown command: " << command << std::endl;
                std::cout << std::endl;
            }
            std::cout << "Commands: " << std::endl;
            std::cout << "  quit      Exits the program" << std::endl;
            std::cout << "  cacheSize Total number of packets in cache" << std::endl;
            std::cout << "  help      Prints this message" << std::endl;
        }
    }
    //logger.debug("Number of packets in capped collection: "
    //    + std::to_string(packetCache.getTotalNumberOfPackets()));
    //logger.debug("Final packet queue size is: "
    //           + std::to_string(dps.mDataPacketQueue.size()));
    logger.debug("Stopping services...");
    service.stop();
    logger.debug("Program finished");
    return EXIT_SUCCESS;
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
/// Read the program options from the command line
std::string parseOptions(int argc, char *argv[])
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
    boost::program_options::notify(vm); //option::notify(vm); 

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

ProgramOptions parseInitializationFile(const std::string &iniFile)
{
    ProgramOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    //------------------------------------------------------------------------//
    const std::string section = "PacketCache";
    options.proxyServiceName = propertyTree.get<std::string>
                               (section + ".proxyServiceName", "");
    if (options.proxyServiceName.empty())
    {
        throw std::invalid_argument("proxyServiceName not set");
    }
    options.maxPackets = propertyTree.get<int> (section + ".maxPackets",
                                                options.maxPackets);

    auto verbosity = propertyTree.get<int>
                     (section + ".verbose",
                      static_cast<int> (options.mVerbosity));
    verbosity = std::min(std::max(0, verbosity), 4); 
    options.mVerbosity = static_cast<UMPS::Logging::Level> (verbosity);

    auto logFileDirectory = propertyTree.get<std::string>
                            (section + ".logFileDirectory",
                            options.mLogFileDirectory);
    if (logFileDirectory.empty())
    {
        logFileDirectory = std::string{std::filesystem::current_path()};
    }
    else
    {
        if (!std::filesystem::exists(logFileDirectory))
        {
            if (!std::filesystem::create_directories(logFileDirectory))
            {
                throw std::runtime_error("Failed to create log file directory: "
                                       + logFileDirectory);
            }
        }
    }
    options.mLogFileDirectory = logFileDirectory;
    //------------------------------ Operator --------------------------------//
    UCI::RequestorOptions requestOptions;
    requestOptions.parseInitializationFile(iniFile);
    options.operatorAddress = requestOptions.getRequestOptions().getAddress();
    options.mZAPOptions = requestOptions.getRequestOptions().getZAPOptions();
    options.mConnectionInformationRequestOptions = requestOptions;
    options.mConnectionInformationRequestOptions.setZAPOptions(
        options.mZAPOptions);
    //------------------------------- Options ---------------------------------//
    options.mPacketCacheReplyOptions.setZAPOptions(options.mZAPOptions);    
/*
    options.operatorAddress = propertyTree.get<std::string>
        ("uOperator.ipAddress", options.operatorAddress);
    if (isEmpty(options.operatorAddress))
    {
        throw std::runtime_error("Operator address not set");
    }
    options.mZAPOptions
        = UMPS::Modules::Operator::readZAPClientOptions(propertyTree);
*/
    return options;
}
