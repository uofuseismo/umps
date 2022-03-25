#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <thread>
#include <zmq.hpp>
#include "umps/proxyServices/incrementer/replier.hpp"
#include "umps/proxyServices/incrementer/replierOptions.hpp"
#include "umps/proxyServices/incrementer/counter.hpp"
#include "umps/proxyServices/incrementer/itemsRequest.hpp"
#include "umps/proxyServices/incrementer/itemsResponse.hpp"
#include "umps/proxyServices/incrementer/incrementRequest.hpp"
#include "umps/proxyServices/incrementer/incrementResponse.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/reply.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::ProxyServices::Incrementer;
namespace URouterDealer = UMPS::Messaging::RouterDealer;
namespace UCI = UMPS::Services::ConnectionInformation;

class Replier::ReplierImpl
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
        mReplier= std::make_unique<URouterDealer::Reply> (context, mLogger);
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
        if (mLogger->getLevel() >= UMPS::Logging::Level::DEBUG)
        {
            mLogger->debug("ReplierImpl::callback: Message of type: "
                         + messageType 
                         + " with length: " + std::to_string(length)
                         + " bytes was received.  Processing...");
        }
        ItemsRequest itemsRequest;
        IncrementRequest incrementRequest;
        if (messageType == incrementRequest.getMessageType())
        {
            auto response = std::make_unique<IncrementResponse> (); 
            // Unpack the request
            std::string itemName;
            try
            {
                auto messagePtr = static_cast<const char *> (messageContents);
                incrementRequest.fromMessage(messagePtr, length);
                itemName = incrementRequest.getItem();
            }
            catch (const std::exception &e) 
            {
                mLogger->error("Request serialization failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response;
            }
            // Set the identifier to help out the recipient
            response->setIdentifier(incrementRequest.getIdentifier());
            // Process the request
            try
            {
                auto nextValue = mCounter->getNextValue(itemName);
                response->setValue(nextValue);
                response->setReturnCode(ReturnCode::SUCCESS);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Incrementer failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
            }
            return response;
        }
        else if (messageType == itemsRequest.getMessageType())
        {
            auto response = std::make_unique<ItemsResponse> ();
            try
            {
                auto messagePtr = static_cast<const char *> (messageContents);
                itemsRequest.fromMessage(messagePtr, length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Request serialization failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response;
            }
            response->setIdentifier(itemsRequest.getIdentifier());
            try
            {
                auto items = mCounter->getItems();
                response->setItems(items);
                response->setReturnCode(ReturnCode::SUCCESS);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Incrementer getItems failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
            }
            return response;
        }
        else
        {
            mLogger->error("Expecting message type: "
                         + itemsRequest.getMessageType() + " or "
                         + incrementRequest.getMessageType()
                         + " but received: " + messageType);
        }
        auto response = std::make_unique<ItemsResponse> ();
        response->setReturnCode(ReturnCode::INVALID_MESSAGE);
        return response;
    }
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
    std::shared_ptr<Counter> mCounter{nullptr};
    std::thread mReplierThread;
    URouterDealer::ReplyOptions mReplyOptions;
    ReplierOptions mOptions;
    bool mInitialized = false;
};

/// Constructor
Replier::Replier() :
    pImpl(std::make_unique<ReplierImpl> (nullptr, nullptr))
{
}

/// C'tor
Replier::Replier(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierImpl> (nullptr, logger))
{
}

/// C'tor
Replier::Replier(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ReplierImpl> (context, logger))
{
}

/// Destructor
Replier::~Replier() = default;

/// Start the service
void Replier::start()
{
    if (!isInitialized()){throw std::runtime_error("Replier not initialized");}
    pImpl->start();
}

/// Is it running?
bool Replier::isRunning() const noexcept
{
    return pImpl->mReplier->isRunning();
}

/// Stop the service
void Replier::stop()
{
    pImpl->stop();
}

/// Initialize the class
void Replier::initialize(
    const ReplierOptions &options,
    std::shared_ptr<Counter> &counter)
{
    if (!counter->isInitialized())
    {
        throw std::invalid_argument("Counter collection not initialized");
    }
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mCounter = counter;
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
bool Replier::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Connection information
UCI::SocketDetails::Reply Replier::getSocketDetails() const
{
    return pImpl->mReplier->getSocketDetails();
}

