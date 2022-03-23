#include <iostream>
#include <chrono>
#include <set>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include "umps/services/incrementer/service.hpp"
#include "umps/services/incrementer/options.hpp"
#include "umps/services/incrementer/counter.hpp"
#include "umps/services/incrementer/incrementRequest.hpp"
#include "umps/services/incrementer/incrementResponse.hpp"
#include "umps/services/incrementer/itemsRequest.hpp"
#include "umps/services/incrementer/itemsResponse.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/services/incrementer/options.hpp"
#include "umps/services/incrementer/counter.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/authentication/service.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Services::Incrementer;
namespace URequestRouter = UMPS::Messaging::RequestRouter;
namespace UAuth = UMPS::Authentication;

class Service::ServiceImpl
{
public:
    /// Constructors
    ServiceImpl() = delete;
    ServiceImpl(std::shared_ptr<zmq::context_t> context,
                std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
        }
        else
        {
            mContext = context;
        }
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        else
        {
            mLogger = logger;
        }
        mRouter = std::make_unique<URequestRouter::Router> (mContext, mLogger);
        //mReplier = std::make_unique<Reply<T>> (mContext, mLogger);
    }
    /// The callback to handle incrementer requests
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *messageContents, const size_t length) noexcept
    {
        if (mLogger->getLevel() >= UMPS::Logging::Level::DEBUG)
        {
            mLogger->debug("ServiceImpl::callback: Message of type: "
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
                auto nextValue = mCounter.getNextValue(itemName);
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
            // 
             
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
    /// Stops the proxy and authenticator and joins threads
    void stop()
    {
        if (mRouter->isRunning()){mRouter->stop();}
        if (mAuthenticatorService->isRunning()){mAuthenticatorService->stop();}
        if (mProxyThread.joinable()){mProxyThread.join();}
        if (mAuthenticatorThread.joinable()){mAuthenticatorThread.join();}
    }
    /// Starts the proxy and authenticator and creates threads
    void start()
    {
        stop();
#ifndef NDEBUG
        assert(mRouter->isInitialized());
#endif
        mProxyThread = std::thread(&URequestRouter::Router::start,
                                   &*mRouter);
        mAuthenticatorThread = std::thread(&UAuth::Service::start,
                                           &*mAuthenticatorService);
    }
    /// Destructor
    ~ServiceImpl()
    {
        stop();
    }
///private:
    std::shared_ptr<zmq::context_t> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<URequestRouter::Router> mRouter{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    Counter mCounter; 
    Options mOptions;
    std::string mName;
    std::thread mProxyThread;
    std::thread mAuthenticatorThread;
    // Where clients can access this service
    ConnectionInformation::Details mConnectionDetails;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{0};
    bool mRunning = false;
    //std::atomic<bool> mRunning{false};
    bool mInitialized = false;
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> (nullptr, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger))
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
                 std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger))
{
}

/// Destructor
Service::~Service() = default;

/// Is the service running
bool Service::isRunning() const noexcept
{
    return pImpl->mRouter->isRunning();
}

/*
/// Initialize
void Service::initialize(const Options &options)
{
    stop(); // Ensure the service is stopped
    // Create the counter
    auto name = parameters.getName();
    auto initialValue = parameters.getInitialValue();
    auto increment = parameters.getIncrement();
    Counter counter;
    counter.initialize(name, initialValue, increment);
    // Initialize the socket - Step 1: Initialize options
    Messaging::RequestRouter::RouterOptions routerOptions;
    auto clientAccessAddress = parameters.getClientAccessAddress();
    routerOptions.setZAPOptions(parameters.getZAPOptions());
    routerOptions.setAddress(clientAccessAddress);
    routerOptions.setCallback(std::bind(&ServiceImpl::callback,
                                        &*this->pImpl,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
    // Add the message types
    std::unique_ptr<UMPS::MessageFormats::IMessage> requestType
        = std::make_unique<Request> (); 
    //routerOptions.addMessageFormat(requestType);
    pImpl->mRouter->initialize(routerOptions); 
    // Save the incrementer service name
    pImpl->mName = name;
    // Create the connection details
    pImpl->mConnectionDetails.setConnectionType(
        ConnectionInformation::ConnectionType::SERVICE);
    pImpl->mConnectionDetails.setName(pImpl->mName);
    pImpl->mConnectionDetails.setSocketDetails(
        pImpl->mRouter->getSocketDetails());
    // Done
    pImpl->mInitialized = true;
}

/// Is the service initialized?
bool Service::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Runs the service
void Service::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Service not initialized");
    }
    pImpl->mLogger->debug("Beginning service " + pImpl->mName + "...");
    pImpl->start();
}

/// Gets the service name
std::string Service::getName() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mName;
}

/// Gets the request address
std::string Service::getRequestAddress() const
{
    if (!isRunning()){throw std::runtime_error("Service is not running");}
    return pImpl->mConnectionDetails.getRouterSocketDetails().getAddress();
}

/// Stop the service
void Service::stop()
{
    pImpl->stop();
    //pImpl->mRouter->stop();
    //pImpl->mCounter.reset();
}

/// Get connection info
UMPS::Services::ConnectionInformation::Details
    Service::getConnectionDetails() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mConnectionDetails;
}
*/
