#include <chrono>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/services/command/localService.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Services::Command;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class LocalService::LocalServiceImpl
{
public:
    /// C'tor
    LocalServiceImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                     std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        // Make the logger
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mRouter = std::make_unique<URequestRouter::Router> (context, mLogger);
    }
    /// Destructor
    ~LocalServiceImpl()
    {
        stop();
    }
    /// Starts the proxy 
    void start()
    {
        stop();
#ifndef NDEBUG
        assert(mRouter->isInitialized());
#endif
        mProxyThread = std::thread(&URequestRouter::Router::start,
                                   &*mRouter);
    }
    /// Stops the proxy and authenticator and joins threads
    void stop()
    {
        if (mRouter->isRunning()){mRouter->stop();}
        if (mProxyThread.joinable()){mProxyThread.join();}
    }

///private:
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<UMPS::Messaging::RequestRouter::Router> mRouter{nullptr};
    UCI::Details mConnectionDetails;
    std::thread mProxyThread;
    const std::string mName{"LocalCommand"};
    bool mInitialized{false};
};

/// C'tor
LocalService::LocalService() :
    pImpl(std::make_unique<LocalServiceImpl> (nullptr, nullptr))
{
}

LocalService::LocalService(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<LocalServiceImpl> (nullptr, logger))
{
}

LocalService::LocalService(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<LocalServiceImpl> (context, nullptr))
{
}

LocalService::LocalService(std::shared_ptr<UMPS::Messaging::Context> &context,
                           std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<LocalServiceImpl> (context, logger))
{
}

/// Initialized?
bool LocalService::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Is the service running?
bool LocalService::isRunning() const noexcept
{
    return pImpl->mRouter->isRunning();
}

/// Service name
std::string LocalService::getName() const
{
    return pImpl->mName;
}

/// Connection details
UCI::Details LocalService::getConnectionDetails() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mConnectionDetails;
}

/// Gets the request address
std::string LocalService::getRequestAddress() const
{
    if (!isRunning()){throw std::runtime_error("Service is not running");}
    return pImpl->mConnectionDetails.getRouterSocketDetails().getAddress();
}

/// Stop the service
void LocalService::stop()
{
    pImpl->stop();
}

/// Runs the service
void LocalService::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Service not initialized");
    }
    pImpl->mLogger->debug("Beginning " + getName() + " service...");
    pImpl->start();
}

/// Destructor
LocalService::~LocalService() = default;
