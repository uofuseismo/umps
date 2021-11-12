#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/broadcasts/earthworm/traceBuf2.hpp"
#include "umps/broadcasts/earthworm/waveRing.hpp"
#include "umps/services/connectionInformation/getConnections.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "private/isEmpty.hpp"

namespace UServices = UMPS::Services;

struct ProgramOptions
{
    std::string earthwormParametersDirectory
        = "/opt/earthworm/run_working/params/";
    std::string earthwormInstallation = "INST_UNKNOWN"; 
    std::string earthwormWaveRingName = "WAVE_RING";
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPackets";
    std::string heartbeatBroadcastName = "Heartbeat";
    std::filesystem::path logFileDirectory = "logs";
    int earthwormWait = 0;
    std::chrono::seconds heartBeatInterval{30};
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
    // Get the connection details
    logger.info("Getting available services...");
    std::vector<UServices::ConnectionInformation::Details> connectionDetails;
    try
    {
        connectionDetails = UServices::ConnectionInformation::getConnections(
                                 options.operatorAddress);
    }
    catch (const std::exception &e)
    {
        logger.error("Error getting services: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    // Connect so that I may publish to appropriate broadcast - e.g., DataPacket

    // Attach to the wave ring
    logger.info("Attaching to earthworm ring: "
              + options.earthwormWaveRingName);
    setenv("EW_PARAMS", options.earthwormParametersDirectory.c_str(), true);
    setenv("EW_INSTALLATION", options.earthwormInstallation.c_str(), true);
    UMPS::Broadcasts::Earthworm::WaveRing waveRing;
    try
    {
        waveRing.connect(options.earthwormWaveRingName, options.earthwormWait);
    }
    catch (const std::exception &e)
    {
        logger.error("Error attaching to wave ring: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    // Forward messages from earthworm to UMPS indefinitely
    auto lastHeartBeat = std::chrono::high_resolution_clock::now(); 
    for (int i = 0; i < 10; ++i)
    {
        // Read from the earthworm ring
        waveRing.read();
        auto nMessages = waveRing.getNumberOfTraceBuf2Messages();
        auto traceBuf2MessagesPtr = waveRing.getTraceBuf2MessagesPointer();
        // Now broadcast the tracebufs as datapacket messages
        for (int iMessage = 0; iMessage < nMessages; ++iMessage)
        {
            // Send it
            try
            {
                auto dataPacket = traceBuf2MessagesPtr[iMessage].toDataPacket();
            }
            catch (const std::exception &e)
            {
                logger.error(e.what());
            }
        }
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
    //waveRing.connect( 0);
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
