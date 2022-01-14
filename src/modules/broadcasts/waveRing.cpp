#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#ifndef NDEBUG
#include <cassert>
#endif
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/broadcasts/earthworm/traceBuf2.hpp"
#include "umps/broadcasts/earthworm/waveRing.hpp"
#include "umps/broadcasts/dataPacket/publisher.hpp"
#include "umps/broadcasts/dataPacket/publisherOptions.hpp"
#include "umps/services/connectionInformation/getConnections.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "private/isEmpty.hpp"

namespace UAuth = UMPS::Authentication;
namespace UServices = UMPS::Services;

struct ProgramOptions
{
    UAuth::ZAPOptions mZAPOptions;
    std::string earthwormParametersDirectory
        = "/opt/earthworm/run_working/params/";
    std::string earthwormInstallation = "INST_UNKNOWN"; 
    std::string earthwormWaveRingName = "WAVE_RING";
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPacket";
    std::string heartbeatBroadcastName = "Heartbeat";
    std::filesystem::path logFileDirectory = "logs";
    int earthwormWait = 0;
    std::chrono::seconds heartBeatInterval{30};
};

class BroadcastPackets
{
public:
    BroadcastPackets(
        std::shared_ptr<UMPS::Broadcasts::DataPacket::Publisher> &publisher,
        std::shared_ptr<UMPS::Broadcasts::Earthworm::WaveRing> &waveRing,
        std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mPublisher(publisher),
        mWaveRing(waveRing),
        mLogger(logger)
    {
    }
    [[nodiscard]] bool keepRunning() const
    {
        std::scoped_lock lock(mMutex);
        return mKeepRunning; 
    }
    void stop()
    {
        std::scoped_lock lock(mMutex);
        mKeepRunning = false;
    }
    void run()
    {
        if (!mWaveRing->isConnected())
        {
            throw std::runtime_error("Wave ring not yet connected");
        }
        if (!mPublisher->isInitialized())
        { 
            throw std::runtime_error("Publisher not yet initialized");
        }
        mWaveRing->flush();
        constexpr std::chrono::seconds oneSecond{1}; 
        while (keepRunning())
        {
            auto startClock = std::chrono::high_resolution_clock::now();
            // Read from the earthworm ring
            mWaveRing->read();
            auto nMessages = mWaveRing->getNumberOfTraceBuf2Messages();
            auto traceBuf2MessagesPtr
                = mWaveRing->getTraceBuf2MessagesPointer();
            // Now broadcast the tracebufs as datapacket messages
            int nSent = 0;
            for (int iMessage = 0; iMessage < nMessages; ++iMessage)
            {
                // Send it
                try
                {
                    auto dataPacket
                        = traceBuf2MessagesPtr[iMessage].toDataPacket();
                    mPublisher->send(dataPacket);
                    nSent = nSent + 1;
                }
                catch (const std::exception &e)
                {
                    mLogger->error(e.what());
                }
            }
            auto endClock = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>
                            (endClock - startClock);
            if (duration < oneSecond)
            {
                auto wait = oneSecond - duration;
                std::this_thread::sleep_for(wait);
            }
            //std::cout << "Sent: " << nSent << std::endl;
        }
        mLogger->info("Broadcast thread is terminating");
    }
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Broadcasts::DataPacket::Publisher> mPublisher;
    std::shared_ptr<UMPS::Broadcasts::Earthworm::WaveRing> mWaveRing;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    bool mKeepRunning = true;
};

std::string parseCommandLineOptions(int argc, char *argv[]);
ProgramOptions parseInitializationFile(const std::string &iniFile);

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
        options = parseInitializationFile(iniFile);
    }
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Create the application's logger
    constexpr int hour = 0;
    constexpr int minute = 0;
    auto waveRingLogFileName = options.logFileDirectory / "waveRing.log";
    UMPS::Logging::SpdLog logger;
    logger.initialize("WaveRing",
                      waveRingLogFileName,
                      UMPS::Logging::Level::DEBUG,
                      hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::SpdLog> (logger);
    // Get the connection details
    logger.info("Getting available services...");
    std::vector<UServices::ConnectionInformation::Details> connectionDetails;
    try
    {
        connectionDetails = UServices::ConnectionInformation::getConnections(
                                 options.operatorAddress,
                                 options.mZAPOptions);
    }
    catch (const std::exception &e)
    {
        logger.error("Error getting services: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    // Connect so that I may publish to appropriate broadcast - e.g., DataPacket
    std::string packetAddress;
    for (const auto &connectionDetail : connectionDetails)
    {
        if (connectionDetail.getName() == options.dataBroadcastName)
        {
            auto proxySocketDetails = connectionDetail.getProxySocketDetails();
            auto frontendSocketDetails
                = proxySocketDetails.getXSubscriberFrontend();
            packetAddress = frontendSocketDetails.getAddress();
            break;
        }
    }
    if (packetAddress.empty())
    {
        logger.error("Failed to find " + options.dataBroadcastName 
                   + " broadcast");
        return EXIT_FAILURE;
    }
    else
    {
        logger.info("Will connect to " + options.dataBroadcastName
                  + " at " + packetAddress); 
    }
    // Connect to proxy
    UMPS::Broadcasts::DataPacket::PublisherOptions publisherOptions;
    publisherOptions.setAddress(packetAddress);
    publisherOptions.setZAPOptions(options.mZAPOptions);
    auto publisher
        = std::make_shared<UMPS::Broadcasts::DataPacket::Publisher> (loggerPtr);
    publisher->initialize(publisherOptions);
#ifndef NDEBUG
    assert(publisher->isInitialized());
#endif
    // Attach to the wave ring
    logger.info("Attaching to earthworm ring: "
              + options.earthwormWaveRingName);
    setenv("EW_PARAMS", options.earthwormParametersDirectory.c_str(), true);
    setenv("EW_INSTALLATION", options.earthwormInstallation.c_str(), true);
    auto waveRing
        = std::make_shared<UMPS::Broadcasts::Earthworm::WaveRing> (loggerPtr);
    try
    {
        waveRing->connect(options.earthwormWaveRingName, options.earthwormWait);
    }
    catch (const std::exception &e)
    {
        logger.error("Error attaching to wave ring: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    waveRing->flush();
    BroadcastPackets broadcastPackets(publisher, waveRing, loggerPtr);
    std::thread broadcastThread(&BroadcastPackets::run, &broadcastPackets);

    while (true)
    {
        std::string command;
        std::cout << "broadcastWaveRing$";
        std::cin >> command;
        if (command == "quit")
        {
            broadcastPackets.stop();
            break;
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
            std::cout << "  quit  Exits the program" << std::endl;
            std::cout << "  help  Prints this message" << std::endl;
        }
    }
/*
    // Forward messages from earthworm to UMPS indefinitely
    auto lastHeartBeat = std::chrono::high_resolution_clock::now(); 
    for (int i = 0; i < 10; ++i)
    {
        // Read from the earthworm ring
        waveRing->read();
        auto nMessages = waveRing->getNumberOfTraceBuf2Messages();
        auto traceBuf2MessagesPtr = waveRing->getTraceBuf2MessagesPointer();
        // Now broadcast the tracebufs as datapacket messages
        int nSent = 0;
        for (int iMessage = 0; iMessage < nMessages; ++iMessage)
        {
            // Send it
            try
            {
                auto dataPacket = traceBuf2MessagesPtr[iMessage].toDataPacket();
                publisher->send(dataPacket);
                nSent = nSent + 1;
            }
            catch (const std::exception &e)
            {
                logger.error(e.what());
            }
        }
        //std::cout << "Sent: " << nSent << std::endl;
        // Is it time to send a heartbeat?
        auto newHeartBeat = std::chrono::high_resolution_clock::now();
        auto heartBeatDuration
            = std::chrono::duration_cast<std::chrono::seconds>
             (newHeartBeat - lastHeartBeat);
        if (heartBeatDuration > options.heartBeatInterval)
        {
            lastHeartBeat = newHeartBeat;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
*/
    broadcastThread.join(); 
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

/// Parse the ini file
ProgramOptions parseInitializationFile(const std::string &iniFile)
{
    ProgramOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    //------------------------------ General ---------------------------------//
    options.logFileDirectory = propertyTree.get<std::string>
        ("WaveRing.logFileDirectory", options.logFileDirectory.string());
    if (!options.logFileDirectory.empty() &&
        !std::filesystem::exists(options.logFileDirectory))
    {
        std::cout << "Creating log file directory: "
                  << options.logFileDirectory << std::endl;
        if (!std::filesystem::create_directories(options.logFileDirectory))
        {
            throw std::runtime_error("Failed to make log directory");
        }
    }
    //------------------------------ Operator --------------------------------//
    options.operatorAddress = propertyTree.get<std::string>
        ("uOperator.ipAddress", options.operatorAddress);
    if (isEmpty(options.operatorAddress))
    {
        throw std::runtime_error("Operator address not set");
    } 
    options.mZAPOptions
        = UMPS::Modules::Operator::readZAPClientOptions(propertyTree);
/*
    // Security level
    auto securityLevel = static_cast<UAuth::SecurityLevel>
       (propertyTree.get<int> ("uOperator.securityLevel",
                    static_cast<int> (options.mZAPOptions.getSecurityLevel())));
    if (static_cast<int> (securityLevel) < 0 ||
        static_cast<int> (securityLevel) > 4)
    {
        throw std::invalid_argument("Security level must be in range [0,4]");
    }
    // Define ZAP options
    options.mZAPOptions.setGrasslandsClient();
    if (securityLevel == UAuth::SecurityLevel::GRASSLANDS)
    {
        options.mZAPOptions.setGrasslandsClient();
    }
    else if (securityLevel == UAuth::SecurityLevel::STRAWHOUSE)
    {
        options.mZAPOptions.setStrawhouseClient();
    }
    else if (securityLevel == UAuth::SecurityLevel::WOODHOUSE)
    {
        UAuth::Certificate::UserNameAndPassword credentials;
        //options.mZAPOptions.setWoodhouseClient();
    }
    else if (securityLevel == UAuth::SecurityLevel::STONEHOUSE)
    {
        auto serverPublicKeyFile
            = propertyTree.get<std::string> ("uOperator.serverPublicKeyFile");
        auto clientPublicKeyFile
            = propertyTree.get<std::string> ("uOperator.clientPublicKeyFile");
        auto clientPrivateKeyFile
            = propertyTree.get<std::string> ("uOperator.clientPrivateKeyFile");
        UAuth::Certificate::Keys serverKeys, clientKeys;
        serverKeys.loadFromTextFile(serverPublicKeyFile);
        clientKeys.loadFromTextFile(clientPublicKeyFile);
        clientKeys.loadFromTextFile(clientPrivateKeyFile);
        options.mZAPOptions.setStonehouseClient(serverKeys, clientKeys);
    }
    else
    {
        throw std::runtime_error("Unhandled security level");
    }
*/
    //------------------------------ Earthworm -------------------------------//
    // EW_PARAMS environment variable
    options.earthwormParametersDirectory = propertyTree.get<std::string>
        ("Earthworm.ewParams", options.earthwormParametersDirectory);
    if (!std::filesystem::exists(options.earthwormParametersDirectory))
    {   
        throw std::runtime_error("Earthworm parameters directory: " 
                               + options.earthwormParametersDirectory
                               + " does not exist");
    }   
    // EW_INST environment variable
    options.earthwormInstallation = propertyTree.get<std::string>
        ("Earthworm.ewInstallation", options.earthwormInstallation);
    // Earthworm wave ring
    options.earthwormWaveRingName = propertyTree.get<std::string>
        ("Earthworm.waveRingName", options.earthwormWaveRingName);
    // Wait after reading
    options.earthwormWait = propertyTree.get<int> ("Earthworm.wait",
                                                   options.earthwormWait);
    if (options.earthwormWait < 0)
    {   
        std::cerr << "Setting wait time to 0" << std::endl;
        options.earthwormWait = 0;
    }   
    //------------------------------------------------------------------------//
    return options;
}
