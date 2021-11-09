#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>
#include "umps/services/incrementer/service.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/services/incrementer/parameters.hpp"
#include "umps/services/incrementer/counter.hpp"
#include "umps/services/incrementer/response.hpp"
#include "umps/services/incrementer/request.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Services::Incrementer;

class Service::ServiceImpl
{
public:
    /// Constructors
    ServiceImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mRouter(mLogger)
    {
    }
    explicit ServiceImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger),
        mRouter(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
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
        auto response = std::make_unique<Response> ();
        Request request;
        if (messageType == request.getMessageType())
        {
            // Unpack the request
            try
            {
                auto messagePtr = static_cast<const char *> (messageContents);
                request.fromMessage(messagePtr, length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Request serialization failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response;
            }
            // Set the identifier to help out the recipient
            response->setIdentifier(request.getIdentifier());
            // Process the request
            try
            {
                auto nextValue = mCounter.getNextValue();
                response->setValue(nextValue);
                response->setReturnCode(ReturnCode::SUCCESS);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Incrementer failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
            }
        }
        else
        {
            mLogger->error("Expecting message type: " + request.getMessageType()
                         + " but received: " + messageType);
            response->setReturnCode(ReturnCode::INVALID_MESSAGE);
        }
        return response;
    }
///private:
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    Counter mCounter; 
    Parameters mParameters;
    UMPS::Messaging::RequestRouter::Router mRouter;
    std::string mName;
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
    pImpl(std::make_unique<ServiceImpl> ())
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (logger))
{
}

/// Move c'tor
Service::Service(Service &&service) noexcept
{
    *this = std::move(service);
}

/// Move assignment
Service& Service::operator=(Service &&service) noexcept
{
    if (&service == this){return *this;}
    pImpl = std::move(service.pImpl);
    return *this;
}

/// Destructor
Service::~Service() = default;

/// Is the service running
bool Service::isRunning() const noexcept
{
    return pImpl->mRouter.isRunning();
}

/// Initialize
void Service::initialize(const Parameters &parameters)
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
    routerOptions.setEndPoint(clientAccessAddress);
    routerOptions.setCallback(std::bind(&ServiceImpl::callback,
                                        &*this->pImpl,
                                        std::placeholders::_1,
                                        std::placeholders::_2,
                                        std::placeholders::_3));
    // Add the message types
    std::unique_ptr<UMPS::MessageFormats::IMessage> requestType
        = std::make_unique<Request> (); 
    routerOptions.addMessageFormat(requestType);
    pImpl->mRouter.initialize(routerOptions); 
    // Save the incrementer service name
    pImpl->mName = name;
    // Create the connection details
    ConnectionInformation::SocketDetails::Router socketDetails;
    socketDetails.setAddress(pImpl->mRouter.getConnectionString());
    pImpl->mConnectionDetails.setName(pImpl->mName);
    pImpl->mConnectionDetails.setSocketDetails(socketDetails);
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
    pImpl->mRouter.start(); // This should hang until the service is stopped
    pImpl->mLogger->debug("Thread exiting service " + pImpl->mName);
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
    pImpl->mRouter.stop();
    //pImpl->mCounter.reset();
}

/// Get connection info
UMPS::Services::ConnectionInformation::Details
    Service::getConnectionDetails() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mConnectionDetails;
}
