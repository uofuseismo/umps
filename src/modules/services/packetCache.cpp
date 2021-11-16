#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/services/packetCache/cappedCollection.hpp"
#include "umps/services/packetCache/dataRequest.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "private/threadSafeQueue.hpp"

struct ProgramOptions
{
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
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Ask the operator how to subscribe to the datapacket broadcast

    // Now connect to the publisher
    UMPS::Messaging::PublisherSubscriber::Subscriber subscriber(loggerPtr);

    // Create a collection of circular buffers
    UMPS::Services::PacketCache::CappedCollection<double>
        cappedCollection(loggerPtr);
    cappedCollection.initialize(options.maxPackets);
    assert(cappedCollection.isInitialized());
    // Continually read from the dataPacket broadcast
    for (int k = 0; k < 10; ++k)
    {
        // Get a good read on time so we wait a predictable amount
        auto startRead = std::chrono::high_resolution_clock::now();

/*
        // Read the ring
        waveRing.read();
        // Get the tracebuf2 messages
        auto traceBuf2Messages = waveRing.getTraceBuf2Messages();
        if (k == 0){logger.info("Setting first batch of messages");}
        for (const auto &traceBuf2Message : traceBuf2Messages)
        {
            UMPS::MessageFormats::DataPacket<double> packet(traceBuf2Message);
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
*/
        auto endRead = std::chrono::high_resolution_clock::now();
        auto elapsedTime
            = std::chrono::duration<double> (endRead - startRead).count();
        logger.debug("Read and update took: "
                   + std::to_string(elapsedTime) + " (s)");
//        logger.debug("Throughput (packets/second): "
//                   + std::to_string(traceBuf2Messages.size()/elapsedTime));
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
    //------------------------------------------------------------------------//
    options.maxPackets = propertyTree.get<int> ("PacketCache.maxPackets",
                                                options.maxPackets);
    return options;
}
