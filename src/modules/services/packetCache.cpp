#include <iostream>
#include <chrono>
#include <string>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/proxyServices/packetCache/sensorRequest.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
#include "umps/proxyServices/packetCache/reply.hpp"
#include "umps/proxyServices/packetCache/replyOptions.hpp"
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
#include "umps/services/connectionInformation/request.hpp"
#include "umps/services/connectionInformation/requestOptions.hpp"
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
#define DEFAULT_MAXPACKETS 100

struct ProgramOptions
{
    UCI::RequestOptions mConnectionInformationRequestOptions;
    UAuth::ZAPOptions mZAPOptions;
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPacket";
    std::string proxyServiceName;
    std::chrono::milliseconds dataPacketTimeOut{10};
    int maxPackets = 100;
    int dataPacketHighWaterMark = static_cast<int> (DEFAULT_HWM);
};

template<class T = double>
class DataPacketSubscriber
{
public:
/*
    /// C'tor
    DataPacketSubscriber(
        std::shared_ptr<zmq::context_t> context,
        std::shared_ptr<UMPS::Logging::ILog> logger,
        std::shared_ptr<UPacketCache::CappedCollection<T>> cappedCollection) :
        mLogger(logger),
        mCappedCollection(cappedCollection)
    {
        mDataPacketSubscriber = std::make_shared<UPubSub::Subscriber> (context, logger); 

        std::unique_ptr<UMPS::MessageFormats::IMessage> messageType
            = std::make_unique<UMPS::MessageFormats::DataPacket<T>> (); 
        UMPS::MessageFormats::Messages messageTypes;
        messageTypes.add(messageType);
        mSubscriberOptions.setMessageTypes(messageTypes);
        mSubscriberOptions.setHighWaterMark(mHighWaterMark);
        mSubscriberOptions.setTimeOut(mTimeOut);
    }
    DataPacketSubscriber(
        std::shared_ptr<UPacketCache::CappedCollection<T>> &cappedCollection)
    {
        DataPacketSubscriber(nullptr, nullptr, cappedCollection);
    }
    DataPacketSubscriber(
        std::shared_ptr<zmq::context_t> &context,
        std::shared_ptr<UPacketCache::CappedCollection<T>> &cappedCollection)
    {
        DataPacketSubscriber(context, nullptr, cappedCollection);
    }
    DataPacketSubscriber(
        std::shared_ptr<UMPS::Logging::ILog> &logger,
        std::shared_ptr<UPacketCache::CappedCollection<T>> &cappedCollection)
    {
        DataPacketSubscriber(nullptr, logger, cappedCollection);
    }
    void setDataPacketFeedAddress(const std::string &address)
    {
        mSubscriberOptions.setAddress(address);
    }
    void setZAPOptions(const UAuth::ZAPOptions &options)
    {
        mSubscriberOptions.setZAPOptions(options);
    }
    void setTimeOut(const std::chrono::milliseconds &timeOut)
    {
        mSubscriberOptions.setTimeOut(timeOut);
    }
    void setHighWaterMark(const int hwm)
    {
        mSubscriberOptions.setHighWaterMark(hwm);
    }
    void setMaximumNumberOfPackets(const int maxPackets)
    {
    }
    void initialize()
    {
        if (mSubscriber->isInitialized())
        {
            mLogger->warn("Subscriber already initialized");
        }
        mSubscriber->initialize(mSubscriberOptions);
#ifndef NDEBUG
        assert(mSubscriber->isInitialized());
#endif
    }
*/
    /// C'tor
    DataPacketSubscriber(
        std::shared_ptr<UMPS::Logging::ILog> &logger,
        std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Subscriber<T>> &subscriber,
        std::shared_ptr<UPacketCache::CappedCollection<T>> &cappedCollection) :
        mLogger(logger),
        mDataPacketSubscriber(subscriber),
        mCappedCollection(cappedCollection)
    {
    }
    ~DataPacketSubscriber()
    {
        stop();
    }
    /// @brief A thread running this function will read messages off the queue
    ///        and put them into the queue.
    void getPackets()
    {
        while (keepRunning())
        {
            try
            {
                auto dataPacket = mDataPacketSubscriber->receive();
                if (dataPacket == nullptr){continue;} // Time out
                mDataPacketQueue.push(std::move(*dataPacket));
            }
            catch (const std::exception &e)
            {
                mLogger->error("Error receiving packet: "
                             + std::string(e.what()));
                continue;
            }
        }
        mLogger->debug("Broadcast to queue thread has exited");
    }
    /// @brief A thread running this function will read messages from the queue
    ///        and put them into the circular buffer.
    void queueToPacketCache()
    {
        namespace UMF = UMPS::MessageFormats;
        while (keepRunning())
        {
            UMF::DataPacket<T> dataPacket;
            if (mDataPacketQueue.wait_until_and_pop(&dataPacket))
            {
                if (dataPacket.getNumberOfSamples() > 0)
                {
                    try
                    {
                        mCappedCollection->addPacket(std::move(dataPacket)); 
                    }
                    catch (const std::exception &e) 
                    {
                        mLogger->error("Failed to add packet:\n"
                                     + std::string{e.what()});
                    }
                }
            }
        }
        mLogger->debug("Queue to circular buffer thread has exited");
    }
    /// @brief Starts the service
    void start()
    {
        // Start thread to read messages from broadcast and put into queue
        mDataPacketSubscriberThread
           = std::thread(&DataPacketSubscriber::getPackets, this);
        // Start thread to read messages from queue and put into packetCache 
        mQueueToPacketCacheThread
           = std::thread(&DataPacketSubscriber::queueToPacketCache, this);
        // Start thread to read / respond to messages
        if (mReplier != nullptr){mReplier->start();}
    }
    /// @result True indicates the data packet subscriber should keep receiving
    ///         messages and putting the results in the circular buffer.
    [[nodiscard]] bool keepRunning() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mKeepRunning;
    }
    /// 
    void setRunning(const bool running)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = false;
    }
    /// @brief Stops the publisher threads.
    void stop()
    {
        setRunning(false);
        if (mLogger != nullptr){mLogger->debug("Notifying threads to stop...");}
        if (mReplier != nullptr)
        {
            if (mReplier->isRunning()){mReplier->stop();}
        }
        if (mDataPacketSubscriberThread.joinable())
        {
            mDataPacketSubscriberThread.join();
        }
        if (mQueueToPacketCacheThread.joinable())
        {
            mQueueToPacketCacheThread.join();
        }
        if (mResponseThread.joinable())
        {
            mResponseThread.join();
        }
    }
///private:
    mutable std::mutex mMutex;
    std::thread mDataPacketSubscriberThread;
    std::thread mQueueToPacketCacheThread;
    std::thread mResponseThread;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    //std::shared_ptr<UPubSub::Subscriber> mSubscriber;
    std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Subscriber<T>>
        mDataPacketSubscriber;
    std::shared_ptr<UPacketCache::CappedCollection<T>> mCappedCollection; 
    ThreadSafeQueue<UMPS::MessageFormats::DataPacket<T>> mDataPacketQueue;
    UPubSub::SubscriberOptions mSubscriberOptions;
    std::shared_ptr<UPacketCache::Reply<T>> mReplier{nullptr};
    std::chrono::milliseconds mTimeOut{DEFAULT_TIMEOUT};
    int mMaxPackets = DEFAULT_MAXPACKETS;
    int mHighWaterMark = DEFAULT_HWM; 
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
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    // Get the connection details
    logger.info("Getting available services...");
    UCI::Request connectionInformation;
    connectionInformation.initialize(
        options.mConnectionInformationRequestOptions);
    auto dataPacketAddress
        = connectionInformation.getProxyBroadcastBackendDetails(
             options.dataBroadcastName).getAddress();
    auto proxyServiceAddress
        = connectionInformation.getProxyServiceBackendDetails(
             options.proxyServiceName).getAddress();
    // Now connect to datapacket broadcast backend 
    UMPS::ProxyBroadcasts::DataPacket::SubscriberOptions<double>
        dataPacketSubscriberOptions;
    dataPacketSubscriberOptions.setAddress(dataPacketAddress);
    dataPacketSubscriberOptions.setHighWaterMark(
        options.dataPacketHighWaterMark);
    dataPacketSubscriberOptions.setTimeOut(options.dataPacketTimeOut);
    dataPacketSubscriberOptions.setZAPOptions(zapOptions);
    auto dataPacketSubscriber
        = std::make_shared<UMPS::ProxyBroadcasts::DataPacket::Subscriber<double>> ();
    dataPacketSubscriber->initialize(dataPacketSubscriberOptions);
    logger.info("Connected!");

    // Create a collection of circular buffers
    auto cappedCollection
        = std::make_shared<UPacketCache::CappedCollection<double>> (loggerPtr);
    cappedCollection->initialize(options.maxPackets);
#ifndef NDEBUG
    assert(cappedCollection->isInitialized());
#endif

    // Create a replier
    UPacketCache::ReplyOptions replyOptions;
    replyOptions.setAddress(proxyServiceAddress);
    auto replier = std::make_shared<UPacketCache::Reply<double>> (loggerPtr);
    replier->initialize(replyOptions, cappedCollection);

    // Create the struct
    DataPacketSubscriber<double> dps(loggerPtr,
                                     dataPacketSubscriber,
                                     cappedCollection);


//DataPacketSubscriber<double> dps(nullptr, loggerPtr, subscriberOptions, cappedCollection);

    //std::thread subscriberToQueueThread(
    //    &DataPacketSubscriber<double>::startSubscriber, &dps);
    //std::thread queueToCircularBufferThread(
    //    &DataPacketSubscriber<double>::startQueueToCircularBuffer,
    //    &dps);
    //dps.startDataPacketSubscriber();
    //dps.startQueueToCircularBuffer();
    logger.info("Starting the service...");
    dps.start();
    // Monitor commands from stdin
    while (true)
    {

    }
/*
    for (int k = 0; k < 5; ++k)
    {
        // Get a good read on time so we wait a predictable amount
        auto startRead = std::chrono::high_resolution_clock::now();
        auto endRead = std::chrono::high_resolution_clock::now();
        auto elapsedTime
            = std::chrono::duration<double> (endRead - startRead).count();
        //logger.debug("Read and update took: "
        //           + std::to_string(elapsedTime) + " (s)");
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
*/
    logger.info("Number of packets in capped collection: "
        + std::to_string(dps.mCappedCollection->getTotalNumberOfPackets()));
    logger.info("Final packet queue size is: "
              + std::to_string(dps.mDataPacketQueue.size()));
    logger.info("Stopping services...");
    dps.stop();
    //subscriberToQueueThread.join();
    //queueToCircularBufferThread.join();
    logger.info("Program finished");
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
    options.proxyServiceName = propertyTree.get<std::string>
                               ("PacketCache.proxyServiceName", "");
    if (options.proxyServiceName.empty())
    {
        throw std::invalid_argument("proxyServiceName not set");
    }
    options.maxPackets = propertyTree.get<int> ("PacketCache.maxPackets",
                                                options.maxPackets);
    //------------------------------ Operator --------------------------------//
    UCI::RequestOptions requestOptions;
    requestOptions.parseInitializationFile(iniFile);
    options.mConnectionInformationRequestOptions = requestOptions;
    options.operatorAddress = requestOptions.getRequestOptions().getAddress();
    options.mZAPOptions = requestOptions.getRequestOptions().getZAPOptions();
    //------------------------------- Options ---------------------------------//
    
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
