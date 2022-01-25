#include <set>
#include <vector>
#include <functional>
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
#include "umps/logging/stdout.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace URouterDealer = UMPS::Messaging::RouterDealer;

class Reply::ReplyImpl
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
        mResponse = std::make_unique<URouterDealer::Reply> (context, mLogger);
    }
    // Respond to data requests
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        process(const std::string &messageType,
                const void *messageContents,
                const size_t length)
    {
        // Get data
        DataRequest dataRequest;
        if (messageType == dataRequest.getMessageType())
        {
            // Deserialize the message
            PacketCache::DataResponse<double> response;
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
                response.setNames(mCappedCollection->getSensorNames());
            }
            catch (...)
            {
                response.setReturnCode(
                    PacketCache::ReturnCode::ALGORITHM_FAILURE);
            }
            return response.clone();
        }
        // Send something back so they don't wait forever
        SensorResponse response;
        response.setReturnCode(ReturnCode::INVALID_MESSAGE_TYPE);
        return response.clone();
    }
    // Create the reply options
    void createReplyOptions(const ReplyOptions &options)
    {
        mOptions = options;
        mReplyOptions = mOptions.getReplyOptions();
        mReplyOptions.setCallback(std::bind(&ReplyImpl::process,   
                                            &*this,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3));
       
    }

    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<URouterDealer::Reply> mResponse{nullptr};
    std::shared_ptr<CappedCollection<double>> mCappedCollection{nullptr};
    URouterDealer::ReplyOptions mReplyOptions;
    ReplyOptions mOptions;
};

/// Constructor
Reply::Reply() :
    pImpl(std::make_unique<ReplyImpl> (nullptr, nullptr))
{
}

/// C'tor
Reply::Reply(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplyImpl> (nullptr, logger))
{
}

/// Destructor
Reply::~Reply() = default;
