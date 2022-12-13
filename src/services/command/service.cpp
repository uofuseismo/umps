#include <iostream>
#include <thread>
#include <csignal>
#include <filesystem>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/services/command/service.hpp"
#include "umps/services/command/serviceOptions.hpp"
#include "umps/services/command/moduleTable.hpp"
#include "umps/services/command/moduleDetails.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/standardOut.hpp"

using namespace UMPS::Services::Command;
namespace UAuth = UMPS::Authentication;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class Service::ServiceImpl
{
public:
    /// Constructor
    ServiceImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                     const std::shared_ptr<UMPS::Logging::ILog> &logger)
    {
        // Make the logger
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StandardOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mRouter = std::make_unique<URequestRouter::Router> (context, mLogger);
    }
    /// Open the module table
    void openModuleTable(const std::string &fileName,
                         const bool createIfDoesNotExist = true)
    {
        if (mModuleTable.isOpen()){mModuleTable.close();}
        mModuleTable.open(fileName, createIfDoesNotExist);
        mTableFile = fileName;
    } 
    void openModuleTable(const bool createIfDoesNotExist = true)
    {
        auto fileName = mTableFile.string(); 
        openModuleTable(fileName, createIfDoesNotExist);
    }
    /// Destructor
    ~ServiceImpl()
    {
        stop();
        if (mModuleTable.haveModule(mModuleDetails.getName()))
        {
            mModuleTable.deleteModule(mModuleDetails.getName());
        }
        mModuleTable.close();
    }
    /// Starts the proxy 
    void start()
    {
        stop();
#ifndef NDEBUG
        assert(mRouter->isInitialized());
#endif
        // Module exists - check if it is dead
        if (mModuleTable.haveModule(mModuleDetails.getName()))
        {
            mLogger->debug("Module: " + mModuleDetails.getName()
                         + " exists.  Checking if dead...");
            auto existingModuleDetails
                = mModuleTable.queryModule(mModuleDetails.getName());
            auto pid
                = static_cast<pid_t>
                  (existingModuleDetails.getProcessIdentifier());
            auto rc = kill(pid, 0);
            // Module exists and is running
            if (rc == 0)
            {
                throw std::invalid_argument("Process exists");
            }
            else // Module is dead - overwrite it's details
            {
                mModuleTable.updateModule(mModuleDetails);
            }
        }
        else // New module
        {
            mModuleTable.addModule(mModuleDetails);
        }
        // Okay - start it
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
    UMPS::Messaging::RequestRouter::RouterOptions mRouterOptions; 
    ModuleTable mModuleTable;
    ModuleDetails mModuleDetails;
    UCI::Details mConnectionDetails;
    std::thread mProxyThread;
    const std::string mName{"LocalCommand"};
    std::filesystem::path mTableFile
        = std::filesystem::path{ std::string(std::getenv("HOME"))
                        + "/.local/share/UMPS/tables/localModuleTable.sqlite3"};
    bool mInitialized{false};
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> (nullptr, nullptr))
{
}

[[maybe_unused]] Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger))
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ServiceImpl> (context, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context,
                           std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (context, logger))
{
}

/// Initilialize
void Service::initialize(const ServiceOptions &options)
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
    // Initialize the local process table
    constexpr bool createModuleTableIfDoesNotExist = true;
    auto moduleTableFile = options.getModuleTable();
    pImpl->openModuleTable(moduleTableFile, createModuleTableIfDoesNotExist);
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
    ModuleDetails moduleDetails;
    moduleDetails.setName(options.getModuleName());
    //moduleDetails.setProcessIdentifier() // Rely on default
    moduleDetails.setIPCDirectory(options.getIPCDirectory());
    moduleDetails.setApplicationStatus(ApplicationStatus::Running);
    pImpl->mModuleDetails = moduleDetails;
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

/// Service name
std::string Service::getName() const
{
    return pImpl->mName;
}

/// Connection details
UCI::Details Service::getConnectionDetails() const
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

/// Destructor
Service::~Service() = default;
