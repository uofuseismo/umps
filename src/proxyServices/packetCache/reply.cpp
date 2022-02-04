#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <thread>
#include <zmq.hpp>
#include "umps/proxyServices/packetCache/reply.hpp"
#include "umps/proxyServices/packetCache/replyOptions.hpp"
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/proxyServices/packetCache/sensorRequest.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace URouterDealer = UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;

template<class T>
class Reply<T>::ReplyImpl
{
public:
    // C'tor
    ReplyImpl(std::shared_ptr<zmq::context_t> context,
              std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mReplier= std::make_unique<URouterDealer::Reply> (context, mLogger);
    }
    /// Destructor
    ~ReplyImpl()
    {
        stop();
    }
    // Respond to data requests
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *messageContents, const size_t length) noexcept
    {
        // Get data
        DataRequest dataRequest;
        if (messageType == dataRequest.getMessageType())
        {
            mLogger->info("Data request received");
            // Deserialize the message
            PacketCache::DataResponse<T> response;
            try
            {
                dataRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(dataRequest.getIdentifier());
            }
            catch (...)
            {
                response.setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            // Does this SNCL exist in the cache?
            auto name = dataRequest.getNetwork() + "."
                      + dataRequest.getStation() + "."
                      + dataRequest.getChannel() + "."
                      + dataRequest.getLocationCode();
            auto haveSensor = mCappedCollection->haveSensor(name);
            if (haveSensor)
            {
                auto [startTime, endTime] = dataRequest.getQueryTimes();
                try
                {
                    auto packets = mCappedCollection->getPackets(name,
                                                                 startTime,
                                                                 endTime);
                    response.setPackets(packets);
                }
                catch (const std::exception &e)
                {
                    mLogger->error(e.what());
                    response.setReturnCode(ReturnCode::ALGORITHM_FAILURE);
                }
            }
            else
            {
                response.setReturnCode(ReturnCode::NO_SENSOR);
                return response.clone();
            }
            return response.clone();
        }
        // Get sensors
        SensorRequest sensorRequest;
        if (messageType == sensorRequest.getMessageType())
        {
            mLogger->info("Sensor request received");
            SensorResponse response;
            try
            {
                sensorRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(sensorRequest.getIdentifier());
            }
            catch (...)
            {
                response.setReturnCode(
                    PacketCache::ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            // Now get the result
            try
            {
std::cout << "getting names" << std::endl;
                response.setNames(mCappedCollection->getSensorNames());
std::cout << "Got names" << std::endl;
            }
            catch (...)
            {
                response.setReturnCode(
                    PacketCache::ReturnCode::ALGORITHM_FAILURE);
            }
            return response.clone();
        }
        // Send something back so they don't wait forever
        mLogger->error("Unhandled message type: " + messageType);
        SensorResponse response;
        response.setReturnCode(ReturnCode::INVALID_MESSAGE_TYPE);
        return response.clone();
    }
    // Create the reply options
/*
    void createReplyOptions(const ReplyOptions &options)
    {
        mOptions = options;
        mReplyOptions = mOptions.getReplyOptions();
        mReplyOptions.setCallback(std::bind(&ReplyImpl::callback,
                                            &*this,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3));
       
    }
*/
    /// Stops the reply service
    void stop()
    {
        if (mReplier != nullptr){mReplier->stop();}
        if (mReplierThread.joinable()){mReplierThread.join();}
    }
    /// Starts the reply service
    void start()
    {
        stop();
        mReplierThread = std::thread(&URouterDealer::Reply::start, &*mReplier);
    }

    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<URouterDealer::Reply> mReplier{nullptr};
    std::shared_ptr<CappedCollection<T>> mCappedCollection{nullptr};
    std::thread mReplierThread;
    URouterDealer::ReplyOptions mReplyOptions;
    ReplyOptions mOptions;
    bool mInitialized = false;
};

/// Constructor
template<class T>
Reply<T>::Reply() :
    pImpl(std::make_unique<ReplyImpl> (nullptr, nullptr))
{
}

/// C'tor
template<class T>
Reply<T>::Reply(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplyImpl> (nullptr, logger))
{
}

/// C'tor
template<class T>
Reply<T>::Reply(std::shared_ptr<zmq::context_t> &context,
                std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplyImpl> (context, logger))
{
}

/// Destructor
template<class T>
Reply<T>::~Reply() = default;

/// Start the service
template<class T>
void Reply<T>::start()
{
    if (!isInitialized()){throw std::runtime_error("Reply not initialized");}
    pImpl->start(); //mReplier->start();
}

/// Is it running?
template<class T>
bool Reply<T>::isRunning() const noexcept
{
    return pImpl->mReplier->isRunning();
}

/// Stop the service
template<class T>
void Reply<T>::stop()
{
    pImpl->stop(); //mReplier->stop();
}

/// Initialize the class
template<class T>
void Reply<T>::initialize(
    const ReplyOptions &options,
    std::shared_ptr<CappedCollection<T>> &cappedCollection)
{
    if (!cappedCollection->isInitialized())
    {
        throw std::invalid_argument("Capped collection not initialized");
    }
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    } 
    pImpl->mCappedCollection = cappedCollection;
    pImpl->mOptions = options;
    pImpl->mReplyOptions = pImpl->mOptions.getReplyOptions();
    pImpl->mReplyOptions.setCallback(std::bind(&ReplyImpl::callback,
                                               &*this->pImpl,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3));
    //pImpl->createReplyOptions(options);
    pImpl->mReplier->initialize(pImpl->mReplyOptions);
    pImpl->mInitialized = true;
}

/// Initialized?
template<class T>
bool Reply<T>::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Connection information
template<class T>
UCI::SocketDetails::Reply Reply<T>::getSocketDetails() const
{
    return pImpl->mReplier->getSocketDetails();
}

///--------------------------------------------------------------------------///
///                          Template Instantiation                          ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyServices::PacketCache::Reply<double>;
template class UMPS::ProxyServices::PacketCache::Reply<float>;
