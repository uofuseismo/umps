#include <chrono>
#include <thread>
#include <filesystem>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/services/command/localService.hpp"
#include "umps/services/command/localServiceOptions.hpp"
#include "umps/services/command/localModuleTable.hpp"
#include "umps/services/command/localModuleDetails.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::Services::Command;
namespace UAuth = UMPS::Authentication;
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
        constexpr bool createIfDoesNotExist = true;
        mLocalModuleTable.open(createIfDoesNotExist);
    }
    /// Destructor
    ~LocalServiceImpl()
    {
        stop();
        if (mLocalModuleTable.haveModule(mLocalModuleDetails))
        {
            mLocalModuleTable.deleteModule(mLocalModuleDetails);
        }
        mLocalModuleTable.close();
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
        mLocalModuleTable.addModule(mLocalModuleDetails);
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
    UMPS::Messaging::RequestRouter::RouterOptions mRouterOptions; 
    LocalModuleTable mLocalModuleTable;
    LocalModuleDetails mLocalModuleDetails;
    UCI::Details mConnectionDetails;
    std::thread mProxyThread;
    const std::string mName{"LocalCommand"};
    //std::filesystem::path mTableFile 
    //    = std::filesystem::path{ std::string(std::getenv("HOME"))
    //                    + "/.local/share/UMPS/tables/localModuleTable.sqlite3"};
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

/// Initilialize
void LocalService::initialize(const LocalServiceOptions &options)
{
    if (!options.haveCallback())
    {
        throw std::invalid_argument("Callback not set");
    }
    if (!options.haveModuleName())
    {
        throw std::invalid_argument("Module name not set");
    }
    stop(); // Ensure the service is stopped
    // Initialize the socket
    auto address = options.getAddress();
    pImpl->mRouterOptions.setAddress(address);
    pImpl->mRouterOptions.setCallback(options.getCallback());
    UAuth::ZAPOptions zapOptions;
    zapOptions.setGrasslandsServer();
    pImpl->mRouterOptions.setZAPOptions(zapOptions);
    // Initialize / connect
    pImpl->mRouter->initialize(pImpl->mRouterOptions);
    // Get the connection details
    auto socketDetails = pImpl->mRouter->getSocketDetails();
    pImpl->mConnectionDetails.setName(getName());
    pImpl->mConnectionDetails.setSocketDetails(socketDetails);
    pImpl->mConnectionDetails.setConnectionType(UCI::ConnectionType::Service);
    pImpl->mConnectionDetails.setSecurityLevel(
        socketDetails.getSecurityLevel());
    // Local module details
    LocalModuleDetails moduleDetails;
    moduleDetails.setName(options.getModuleName());
    //moduleDetails.setProcessIdentifier() // Rely on default
    moduleDetails.setIPCDirectory(options.getIPCDirectory());
    moduleDetails.setApplicationStatus(ApplicationStatus::Running);
    pImpl->mLocalModuleDetails = moduleDetails;
    // Done
    pImpl->mInitialized = true;
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
