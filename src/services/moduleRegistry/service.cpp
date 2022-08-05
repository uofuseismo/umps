#include <iostream>
#include <map>
#include <string>
#include <chrono>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include <functional>
#include "umps/services/moduleRegistry/service.hpp"
#include "umps/services/moduleRegistry/serviceOptions.hpp"
#include "umps/services/moduleRegistry/moduleDetails.hpp"
#include "umps/services/moduleRegistry/registrationRequest.hpp"
#include "umps/services/moduleRegistry/registrationResponse.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/services/service.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/authentication/service.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Services::ModuleRegistry;
namespace URequestRouter = UMPS::Messaging::RequestRouter;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Service::ServiceImpl
{
public:
    /// Constructors
    ServiceImpl() = delete;
    ServiceImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                std::shared_ptr<UMPS::Logging::ILog> logger,
                std::shared_ptr<UAuth::IAuthenticator> authenticator)
    {   
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> (1);
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
        if (authenticator == nullptr)
        {
            mAuthenticator = std::make_shared<UAuth::Grasslands> (mLogger);
        }
        else
        {
            mAuthenticator = authenticator;
        }
        mRouter = std::make_unique<URequestRouter::Router> (mContext, mLogger);
        mAuthenticatorService
            = std::make_unique<UAuth::Service>
              (mContext, mLogger, mAuthenticator);
    }
    /// The callback to handle connection requests
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
        //RegistrationResponse response;
        RegistrationRequest registrationRequest;
        if (messageType == registrationRequest.getMessageType())
        {
            RegistrationResponse response;
            // Unpack the request
            try
            {
                auto messagePtr = static_cast<const char *> (messageContents);
                registrationRequest.fromMessage(messagePtr, length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Reg request serialization failed with: "
                             + std::string(e.what()));
                response.setReturnCode(
                    RegistrationReturnCode::AlgorithmFailure);
                return response.clone();
            }
            auto details = registrationRequest.getModuleDetails();
            if (contains(details.getName()))
            {
                response.setReturnCode(
                    RegistrationReturnCode::Exists);
                return response.clone();
            }
            try
            {
                insert(details);
                response.setReturnCode(RegistrationReturnCode::Success);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Reg request failed with: "
                             + std::string(e.what()));
                response.setReturnCode(
                    RegistrationReturnCode::AlgorithmFailure);
            }
            return response.clone();
        }
        else
        {
            RegistrationResponse response;
            response.setReturnCode(RegistrationReturnCode::InvalidMessage);
            return response.clone();
        }
    }
    /// Contains?
    bool contains(const std::string &name) const noexcept
    {
        return mRegisteredModules.contains(name);
    } 
    bool contains(const ModuleDetails &details) const
    {
        return contains(details.getName());
    }
    /// Insert
    void insert(const ModuleDetails &details)
    {
        if (contains(details))
        {
            throw std::invalid_argument(details.getName() + " already exists");
        }
        mRegisteredModules.insert(std::pair{details.getName(), details});
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
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<URequestRouter::Router> mRouter{nullptr};
    std::unique_ptr<UAuth::Service> mAuthenticatorService{nullptr};
    std::shared_ptr<UAuth::IAuthenticator> mAuthenticator{nullptr};
    UCI::Details mConnectionDetails;
    std::map<std::string, ModuleDetails> mRegisteredModules;
    UMPS::Messaging::RequestRouter::RouterOptions mRouterOptions;
    std::thread mProxyThread;
    std::thread mAuthenticatorThread;
    const std::string mName = ServiceOptions::getName();
    bool mInitialized{false};
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> (nullptr, nullptr, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
                 std::shared_ptr<UAuth::IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger, authenticator))
{
}

/// Destructor
Service::~Service() = default;

/// Initialize
void Service::initialize(const ServiceOptions &options)
{
    if (!options.haveClientAccessAddress())
    {
        throw std::invalid_argument("Client access address not set");
    }
    stop(); // Ensure the service is stopped
    // Clear out the old services and broadcasts
    //pImpl->mConnections.clear();
    pImpl->mRouterOptions.clear();
    // Step 1: Initialize socket
    auto clientAccessAddress = options.getClientAccessAddress();
    pImpl->mRouterOptions.setAddress(clientAccessAddress);
    pImpl->mRouterOptions.setCallback(std::bind(&ServiceImpl::callback,
                                                &*this->pImpl,
                                                std::placeholders::_1,
                                                std::placeholders::_2,
                                                std::placeholders::_3));
    pImpl->mRouterOptions.setZAPOptions(options.getZAPOptions());
    pImpl->mRouter->initialize(pImpl->mRouterOptions);
    // Step 2: Create the connection details
    auto socketDetails = pImpl->mRouter->getSocketDetails();
    pImpl->mConnectionDetails.setName(getName());
    pImpl->mConnectionDetails.setSocketDetails(socketDetails);
    pImpl->mConnectionDetails.setConnectionType(UCI::ConnectionType::Service);
    pImpl->mConnectionDetails.setSecurityLevel(
        socketDetails.getSecurityLevel());
    // Done
    pImpl->mInitialized = true;
}

/// Initialized?
bool Service::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Is the service running?
bool Service::isRunning() const noexcept
{
    return pImpl->mRouter->isRunning();
}

/// Connection details
UMPS::Services::ConnectionInformation::Details
    Service::getConnectionDetails() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mConnectionDetails;
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
}

/// Gets the service name
std::string Service::getName() const
{
    return pImpl->mName;
}

/// Runs the service
void Service::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Service not initialized");
    }
    pImpl->mLogger->debug("Beginning " + getName() + " service...");
    pImpl->start();
}
