#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <thread>
#include <zmq.hpp>
#include "umps/proxyServices/packetCache/replier.hpp"
#include "umps/proxyServices/packetCache/replierOptions.hpp"
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/bulkDataRequest.hpp"
#include "umps/proxyServices/packetCache/bulkDataResponse.hpp"
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

namespace
{

template<typename T>
void performDataRequest(const DataRequest &dataRequest,
                        const CappedCollection<T> &mCappedCollection,
                        DataResponse<T> *response)
{
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
            auto packets = mCappedCollection.getPackets(name,
                                                        startTime,
                                                        endTime);
            response->setPackets(packets);
        }
        catch (const std::exception &e) 
        {
            auto errorMessage = "Query failed with: " + std::string {e.what()};
            response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
            throw std::runtime_error(errorMessage);
        }
   }
   else
   {
       response->setReturnCode(ReturnCode::NO_SENSOR);
   }
}

}

template<class T>
class Replier<T>::ReplierImpl
{
public:
    // C'tor
    ReplierImpl(std::shared_ptr<zmq::context_t> context,
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
        mReplier = std::make_unique<URouterDealer::Reply> (context, mLogger);
    }
    /// Destructor
    ~ReplierImpl()
    {
        stop();
    }
    // Respond to data requests
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *messageContents, const size_t length) noexcept
    {
        // Get data
        mLogger->debug("Request received");
        DataRequest dataRequest;
        if (messageType == dataRequest.getMessageType())
        {
            mLogger->debug("Data request received");
            // Deserialize the message
            DataResponse<T> response;
            try
            {
                dataRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(dataRequest.getIdentifier());
            }
            catch (...)
            {
                mLogger->error("Received invalid data request");
                response.setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            // Does this SNCL exist in the cache?
            try
            {
                performDataRequest(dataRequest,
                                   *mCappedCollection,
                                   &response);
            }
            catch (const std::exception &e)
            {
                mLogger->error(e.what());
            }
            /*
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
                    mLogger->error("Query failed");
                    mLogger->error(e.what());
                    response.setReturnCode(ReturnCode::ALGORITHM_FAILURE);
                    return response.clone();
                }
            }
            else
            {
                response.setReturnCode(ReturnCode::NO_SENSOR);
                return response.clone();
            }
            */
            mLogger->debug("Replying to data request");
            return response.clone();
        }
        // Bulk data request
        BulkDataRequest bulkDataRequest;
        if (messageType == bulkDataRequest.getMessageType())
        {
            mLogger->debug("Bulk data request received");
            // Deserialize the message
            PacketCache::BulkDataResponse<T> response;
            try
            {
                bulkDataRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(bulkDataRequest.getIdentifier());
            }
            catch (...)
            {
                mLogger->error("Received invalid data request");
                response.setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            auto nRequests = bulkDataRequest.getNumberOfDataRequests();
            const auto dataRequests = bulkDataRequest.getDataRequestsPointer();
            for (int iRequest = 0; iRequest < nRequests; ++iRequest)
            {
                DataResponse<T> dataResponse;
                dataResponse.setIdentifier(
                    dataRequests[iRequest].getIdentifier());
                try
                {
                    performDataRequest(dataRequests[iRequest],
                                       *mCappedCollection,
                                       &dataResponse);
                }
                catch (const std::exception &e)
                {
                    mLogger->error(e.what());
                }
                response.addDataResponse(std::move(dataResponse));
            }
            mLogger->debug("Replying to bulk data request");
            return response.clone();
        }
        // Get sensors
        SensorRequest sensorRequest;
        if (messageType == sensorRequest.getMessageType())
        {
            mLogger->debug("Sensor request received");
            SensorResponse response;
            try
            {
                sensorRequest.fromMessage(
                    static_cast<const char *> (messageContents), length);
                response.setIdentifier(sensorRequest.getIdentifier());
            }
            catch (...)
            {
                mLogger->error("Received invalid sensor request");
                response.setReturnCode(
                    PacketCache::ReturnCode::INVALID_MESSAGE);
                return response.clone();
            }
            // Now get the result
            try
            {
                response.setNames(mCappedCollection->getSensorNames());
            }
            catch (...)
            {
                mLogger->error("Failed to set sensor names");
                response.setReturnCode(
                    PacketCache::ReturnCode::ALGORITHM_FAILURE);
            }
            mLogger->debug("Replying to sensor request");
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
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<URouterDealer::Reply> mReplier{nullptr};
    std::shared_ptr<CappedCollection<T>> mCappedCollection{nullptr};
    std::thread mReplierThread;
    URouterDealer::ReplyOptions mReplyOptions;
    ReplierOptions mOptions;
    bool mInitialized = false;
};

/// Constructor
template<class T>
Replier<T>::Replier() :
    pImpl(std::make_unique<ReplierImpl> (nullptr, nullptr))
{
}

/// C'tor
template<class T>
Replier<T>::Replier(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierImpl> (nullptr, logger))
{
}

/// C'tor
template<class T>
Replier<T>::Replier(std::shared_ptr<zmq::context_t> &context,
                    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierImpl> (context, logger))
{
}

/// Destructor
template<class T>
Replier<T>::~Replier() = default;

/// Start the service
template<class T>
void Replier<T>::start()
{
    if (!isInitialized()){throw std::runtime_error("Replier not initialized");}
    pImpl->start(); //mReplier->start();
}

/// Is it running?
template<class T>
bool Replier<T>::isRunning() const noexcept
{
    return pImpl->mReplier->isRunning();
}

/// Stop the service
template<class T>
void Replier<T>::stop()
{
    pImpl->stop(); //mReplier->stop();
}

/// Initialize the class
template<class T>
void Replier<T>::initialize(
    const ReplierOptions &options,
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
    pImpl->mReplyOptions.setCallback(std::bind(&ReplierImpl::callback,
                                               &*this->pImpl,
                                               std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3));
    pImpl->mReplier->initialize(pImpl->mReplyOptions);
#ifndef NDEBUG
    assert(pImpl->mReplier->isInitialized());
#endif
    pImpl->mInitialized = true;
}

/// Initialized?
template<class T>
bool Replier<T>::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Connection information
template<class T>
UCI::SocketDetails::Reply Replier<T>::getSocketDetails() const
{
    return pImpl->mReplier->getSocketDetails();
}

///--------------------------------------------------------------------------///
///                          Template Instantiation                          ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyServices::PacketCache::Replier<double>;
template class UMPS::ProxyServices::PacketCache::Replier<float>;
