#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "urts/services/packetCache/cappedCollection.hpp"
#include "urts/services/packetCache/dataRequest.hpp"
#include "urts/messaging/earthworm/waveRing.hpp"
#include "urts/messageFormats/dataPacket.hpp"
#include "urts/messageFormats/earthworm/traceBuf2.hpp"
#include "urts/logging/spdlog.hpp"
#include "urts/logging/stdout.hpp"

struct ProgramOptions
{
    std::string earthwormParametersDirectory
        = "/opt/earthworm/run_working/params/";
    std::string earthwormInstallation = "INST_UNKNOWN"; 
    std::string earthwormWaveRingName = "WAVE_RING";
    int earthwormWait = 0;
    int maxPackets = 100;
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
    // Create logger now that we know what is desired
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::DEBUG);
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);
    // Create a collection of circular buffers
    URTS::Services::PacketCache::CappedCollection<double>
        cappedCollection(loggerPtr);
    cappedCollection.initialize(options.maxPackets);
    assert(cappedCollection.isInitialized());
    // Initialize earthworm connection
    logger.debug("Setting environment variables...");
    setenv("EW_PARAMS", options.earthwormParametersDirectory.c_str(), true);
    setenv("EW_INSTALLATION", options.earthwormInstallation.c_str(), true);
    logger.debug("Constructing wave ring connection..."); 
    URTS::Messaging::Earthworm::WaveRing waveRing(loggerPtr);
    try
    {
        waveRing.connect(options.earthwormWaveRingName,
                         options.earthwormWait);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    assert(waveRing.isConnected());
    // Flush the ring so we can start from scratch
    //waveRing.flush();
    // Try reading...
    for (int k = 0; k < 10; ++k)
    {
        // Get a good read on time so we wait a predictable amount
        auto startRead = std::chrono::high_resolution_clock::now();

        // Read the ring
        waveRing.read();
        // Get the tracebuf2 messages
        auto traceBuf2Messages = waveRing.getTraceBuf2Messages();
        if (k == 0){logger.info("Setting first batch of messages");}
        for (const auto &traceBuf2Message : traceBuf2Messages)
        {
            URTS::MessageFormats::DataPacket<double> packet(traceBuf2Message);
            try
            {
                cappedCollection.addPacket(std::move(packet));
            }
            catch (const std::exception &e)
            {
                logger.error("Error detected: " + std::string(e.what())
                           + "; skipping...");
            } 
        }
        auto endRead = std::chrono::high_resolution_clock::now();
        auto elapsedTime
            = std::chrono::duration<double> (endRead - startRead).count();
        logger.debug("Read and update took: "
                   + std::to_string(elapsedTime) + " (s)");
        logger.debug("Throughput (packets/second): "
                   + std::to_string(traceBuf2Messages.size()/elapsedTime));
        //sleep(1);
        auto sleepTime = static_cast<int> (1000 - elapsedTime*1000);
        if (sleepTime > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
        }
//std::cout << packet.toJSON(4) << std::endl;
//break;
    }

//std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    options.maxPackets = propertyTree.get<int> ("PacketCache.maxPackets",
                                                options.maxPackets);
    return options;
}
