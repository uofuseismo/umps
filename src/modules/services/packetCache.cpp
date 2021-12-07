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
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/getConnections.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "private/threadSafeQueue.hpp"
#include "private/staticUniquePointerCast.hpp"
#include "private/isEmpty.hpp"

namespace UPacketCache = UMPS::Services::PacketCache;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;

struct ProgramOptions
{
    UMPS::Messaging::Authentication::ZAPOptions mZAPOptions;
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPacket";
    std::chrono::milliseconds dataPacketTimeOut{10};
    int maxPackets = 100;
    int dataPacketHighWaterMark = 4*1024;
};

template<class T = double>
class DataPacketSubscriber
{
public:
    /// C'tor
    DataPacketSubscriber(
        std::shared_ptr<UMPS::Logging::ILog> &logger,
        std::shared_ptr<UPubSub::Subscriber> &subscriber,
        std::shared_ptr<UPacketCache::CappedCollection<T>> &cappedCollection) :
        mLogger(logger),
        mSubscriber(subscriber),
        mCappedCollection(cappedCollection)
    {
    }
    /// @brief Starts the service that reads from the data packet feed and
    ///        puts them into the queue for another thread to process.  
    void startSubscriber()
    {
        namespace UMF = UMPS::MessageFormats;
        mLogger->debug("Subscriber thread starting...");
        while (keepRunning())
        {
            // Read latest message
            //std::unique_ptr<UMPS::MessageFormats::IMessage> message = mSubscriber->receive();
            //if (message == nullptr){continue;} // Possible to timeout
            //auto dataPacket
            //    = static_unique_pointer_cast<UMF::DataPacket<T>> (std::move(message));
            auto dataPacket = static_unique_pointer_cast<UMF::DataPacket<T>>
                              (mSubscriber->receive());
            if (dataPacket == nullptr){continue;}
            // Push it onto the queue
            mDataPacketQueue.push(std::move(*dataPacket));
        }
        // Put one last message in the queue so the other thread can unlock
        // and quit its for loop 
        mLogger->debug("Subscriber thread has exited");
        UMF::DataPacket<T> lastPacket;
        mDataPacketQueue.push(lastPacket);
    }
    /// @brief Starts the service that takes a packet from the queue
    ///        and into the packedCollection.
    void startQueueToCircularBuffer()
    {
        namespace UMF = UMPS::MessageFormats;
        mLogger->debug("Queue to circular buffer thread starting...");
        while (keepRunning())
        {
            UMF::DataPacket<T> dataPacket;
            mDataPacketQueue.wait_and_pop(&dataPacket);
            if (!keepRunning()){break;}
            // Add the packet to the queue
            mCappedCollection->addPacket(std::move(dataPacket));
        }
        mLogger->debug("Queue to circular buffer thread has exited");
    }
    /// @brief Starts the service that receives and fulfills requests for
    ///        data packets from time t0 to time t1.
    void startRequestReceive()
    {
        while (keepRunning())
        {
        }
    }
    /// @result True indicates the data packet subscriber should keep receiving
    ///         messages and putting the results in the circular buffer.
    [[nodiscard]] bool keepRunning() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mKeepRunning;
    }
    /// @brief Stops the publisher threads.
    void stop()
    {
        mLogger->debug("Notifying threads to stop...");
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = false;
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::shared_ptr<UPubSub::Subscriber> mSubscriber;
    std::shared_ptr<UPacketCache::CappedCollection<T>> mCappedCollection; 
    ThreadSafeQueue<UMPS::MessageFormats::DataPacket<T>> mDataPacketQueue;
    bool mKeepRunning = true;
    //bool mRunning = false;
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
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Get the connection details
    logger.info("Getting available services...");
    namespace UCI = UMPS::Services::ConnectionInformation;
    std::vector<UCI::Details> connectionDetails;
    try
    {
        connectionDetails = UCI::getConnections(options.operatorAddress);
    }
    catch (const std::exception &e)
    {
        logger.error("Error getting services: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    // Connect so that I may read from the appropriate broadcast - e.g., DataPacket
    std::string dataPacketAddress;
    for (const auto &connectionDetail : connectionDetails)
    {
        if (connectionDetail.getName() == options.dataBroadcastName)
        {
            auto proxySocketDetails = connectionDetail.getProxySocketDetails();
            auto backendSocketDetails
                = proxySocketDetails.getXPublisherBackend();
            dataPacketAddress = backendSocketDetails.getAddress();
            break;
        }
    }   
    if (dataPacketAddress.empty())
    {
        logger.error("Failed to find " + options.dataBroadcastName 
                   + " broadcast");
        return EXIT_FAILURE;
    }
    else
    {
        logger.info("Will connect to " + options.dataBroadcastName
                  + " at " + dataPacketAddress); 
    }

    // Now connect to the publisher
    std::unique_ptr<UMPS::MessageFormats::IMessage> messageType
        = std::make_unique<UMPS::MessageFormats::DataPacket<double>> ();
    UMPS::Messaging::PublisherSubscriber::SubscriberOptions subscriberOptions;
    UMPS::MessageFormats::Messages messageTypes;
    messageTypes.add(messageType);

    subscriberOptions.setAddress(dataPacketAddress);
    subscriberOptions.setHighWaterMark(options.dataPacketHighWaterMark);
    subscriberOptions.setTimeOut(options.dataPacketTimeOut);
    subscriberOptions.setMessageTypes(messageTypes);
    subscriberOptions.setZAPOptions(zapOptions);

    auto subscriber = std::make_shared<UPubSub::Subscriber> (loggerPtr);
    subscriber->initialize(subscriberOptions);
    assert(subscriber->isInitialized());
    //subscriber.connect(dataPacketAddress);
    //subscriber.addSubscription(messageType);

    // Create a collection of circular buffers
    auto cappedCollection
        = std::make_shared<UPacketCache::CappedCollection<double>> (loggerPtr);
    cappedCollection->initialize(options.maxPackets);
#ifndef NDEBUG
    assert(cappedCollection->isInitialized());
#endif
    // Create the struct
    DataPacketSubscriber<double> dps(loggerPtr, subscriber, cappedCollection);
    std::thread subscriberToQueueThread(
        &DataPacketSubscriber<double>::startSubscriber, &dps);
    std::thread queueToCircularBufferThread(
        &DataPacketSubscriber<double>::startQueueToCircularBuffer,
        &dps);
    // Continually read from the dataPacket broadcast
    for (int k = 0; k < 20; ++k)
    {
        // Get a good read on time so we wait a predictable amount
        auto startRead = std::chrono::high_resolution_clock::now();
/*
int nRecv = 0;
while (true)
{
        auto message = subscriber->receive();
        if (message == nullptr){break;}
 nRecv = nRecv + 1;
 //std::cout << nRecv << " " << elapsedTime << std::endl;
}
std::cout << "received: " << nRecv << std::endl;
*/
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

    loggerPtr->info("Stopping services...");
std::cout << dps.mDataPacketQueue.size() << " " << dps.mCappedCollection->getTotalNumberOfPackets() << std::endl;
    dps.stop();
    subscriberToQueueThread.join();
    queueToCircularBufferThread.join();

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
    //------------------------------ Operator --------------------------------//
    options.operatorAddress = propertyTree.get<std::string>
        ("uOperator.ipAddress", options.operatorAddress);
    if (isEmpty(options.operatorAddress))
    {
        throw std::runtime_error("Operator address not set");
    }
    return options;
}