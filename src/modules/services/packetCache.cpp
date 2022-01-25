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
#include "umps/services/connectionInformation/getConnections.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "private/threadSafeQueue.hpp"
#include "private/staticUniquePointerCast.hpp"
#include "private/isEmpty.hpp"

namespace UPacketCache = UMPS::ProxyServices::PacketCache;
namespace UPubSub = UMPS::Messaging::PublisherSubscriber;
namespace UAuth = UMPS::Authentication;

#define DEFAULT_HWM 4096
#define DEFAULT_TIMEOUT std::chrono::milliseconds{10}
#define DEFAULT_MAXPACKETS 100

struct ProgramOptions
{
    UAuth::ZAPOptions mZAPOptions;
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPacket";
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
            //auto dataPacket = static_unique_pointer_cast<UMF::DataPacket<T>>
            //                  (mSubscriber->receive());
            auto dataPacket = mDataPacketSubscriber->receive();
            if (dataPacket == nullptr){continue;}
            // Push it onto the queue
            mDataPacketQueue.push(std::move(*dataPacket));
        }
        // Put one last message in the queue so the other thread can unlock
        // and quit its for loop 
        mLogger->debug("Data packet subscriber thread has exited");
        //UMF::DataPacket<T> lastPacket;
        //mDataPacketQueue.push(lastPacket);
    }
    /// @brief Starts the service that takes a packet from the queue
    ///        and into the packedCollection.
    void startQueueToCircularBuffer()
    {
        namespace UMF = UMPS::MessageFormats;
        mLogger->debug("Queue to circular buffer thread starting...");
        while (keepRunning())
        {
            /*
            UMF::DataPacket<T> dataPacket;
            mDataPacketQueue.wait_and_pop(&dataPacket);
            if (!keepRunning()){break;}
            // Add the packet to the queue
            if (dataPacket.getNumberOfSamples() > 0)
            {
                mCappedCollection->addPacket(std::move(dataPacket));
            }
            */
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
    /// @brief Starts the service that receives and fulfills requests for
    ///        data packets from time t0 to time t1.
    void startRequestReceive()
    {
        mLogger->debug("Request/response thread starting...");
        while (keepRunning())
        {
           // auto request =  
        }
        mLogger->debug("Request/response thread has exited");
    }
    /// @result True indicates the data packet subscriber should keep receiving
    ///         messages and putting the results in the circular buffer.
    [[nodiscard]] bool keepRunning() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mKeepRunning;
    }
    /// @brief Processes data requests.
    /// @result A response to the request.
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        processDataRequest(const std::string &messageType,
                           const void *messageContents, const size_t length)
    {
        // Get data
        UPacketCache::DataRequest dataRequest;
        if (messageType == dataRequest.getMessageType())
        {
            // Deserialize the message
            UPacketCache::DataResponse<T> response;
            try
            {
                dataRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(dataRequest.getIdentifier());
            }
            catch (...)
            {
                response.setReturnCode(
                   UPacketCache::ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            // Does this SNCL exist in the cache?
            auto name = dataRequest.getNetwork() + "."
                      + dataRequest.getStation() + "."
                      + dataRequest.getChannel() + "."
                      + dataRequest.getLocationCode();
            auto haveSensor = mCappedCollection.haveSensor(name);
            if (haveSensor)
            {
                auto [startTime, endTime] = dataRequest.getQueryTimes();
                try
                {
                    auto packets = mCappedCollection.getPackets(
                        name, startTime, endTime);
                    response.setPackets(packets);
                }
                catch (const std::exception &e)
                {
                    mLogger->error(e.what());
                    response.setReturnCode(
                       UPacketCache::ReturnCode::ALGORITHM_FAILURE);
                }
            }
            else
            {
                response.setReturnCode(UPacketCache::ReturnCode::NO_SENSOR);
                return response.clone();
            }
            return response.clone();
        }
        // Get sensors
        UPacketCache::SensorRequest sensorRequest;
        if (messageType == sensorRequest.getMessageType())
        {
            UPacketCache::SensorResponse response;
            try
            {
                sensorRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(sensorRequest.getIdentifier());
            }
            catch (...)
            {
                response.setReturnCode(
                    UPacketCache::ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            // Now get the result
            try
            {
                response.setNames(mCappedCollection->getSensors());
            }
            catch (...)
            {
                response.setReturnCode(
                    UPacketCache::ReturnCode::ALGORITHM_FAILURE);
            }
            return response.clone();
        }
        // Send something back so they don't wait forever
        UPacketCache::SensorResponse response;
        response.setReturnCode(UPacketCache::ReturnCode::INVALID_MESSAGE_TYPE);
        return response.clone();
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
    //std::shared_ptr<UPubSub::Subscriber> mSubscriber;
    std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Subscriber<T>>
        mDataPacketSubscriber;
    std::shared_ptr<UPacketCache::CappedCollection<T>> mCappedCollection; 
    std::shared_ptr<UPacketCache::Reply> mResponder{nullptr};
    ThreadSafeQueue<UMPS::MessageFormats::DataPacket<T>> mDataPacketQueue;
    UPubSub::SubscriberOptions mSubscriberOptions;
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
    namespace UCI = UMPS::Services::ConnectionInformation;
    std::vector<UCI::Details> connectionDetails;
    try
    {
        connectionDetails = UCI::getConnections(options.operatorAddress,
                                                options.mZAPOptions);
    }
    catch (const std::exception &e)
    {
        logger.error("Error getting services: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    // Connect so that I may read from the appropriate broadcast
    // - e.g., DataPacket
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

/*
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
#ifndef NDEBUG
    assert(subscriber->isInitialized());
#endif
*/

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
    // Create the struct
    DataPacketSubscriber<double> dps(loggerPtr,
                                     dataPacketSubscriber,
                                     cappedCollection);


//DataPacketSubscriber<double> dps(nullptr, loggerPtr, subscriberOptions, cappedCollection);

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
    logger.info("Number of packets in capped collection: "
        + std::to_string(dps.mCappedCollection->getTotalNumberOfPackets()));
    logger.info("Final packet queue size is: "
              + std::to_string(dps.mDataPacketQueue.size()));
    logger.info("Stopping services...");
    dps.stop();
    subscriberToQueueThread.join();
    queueToCircularBufferThread.join();
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
    options.maxPackets = propertyTree.get<int> ("PacketCache.maxPackets",
                                                options.maxPackets);
    //------------------------------ Operator --------------------------------//
    options.operatorAddress = propertyTree.get<std::string>
        ("uOperator.ipAddress", options.operatorAddress);
    if (isEmpty(options.operatorAddress))
    {
        throw std::runtime_error("Operator address not set");
    }
    options.mZAPOptions
        = UMPS::Modules::Operator::readZAPClientOptions(propertyTree);
    return options;
}
