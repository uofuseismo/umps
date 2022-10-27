#include <iostream>
#include <chrono>
#include <mutex>
#include <set>
#include <thread>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/proxyServices/incrementer/service.hpp"
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/proxyServices/incrementer/counter.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/proxyServices/incrementer/counter.hpp"
#include "umps/proxyServices/incrementer/replier.hpp"
#include "umps/proxyServices/incrementer/replierOptions.hpp"
#include "umps/logging/standardOut.hpp"
#include "umps/authentication/service.hpp"
#include "umps/messaging/context.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyServices::Incrementer;
namespace UAuth = UMPS::Authentication;

class Service::ServiceImpl
{
public:
    /// Constructors
    ServiceImpl() = delete;
    ServiceImpl(const std::shared_ptr<UMPS::Messaging::Context> &context,
                const std::shared_ptr<UMPS::Logging::ILog> &logger)
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
            mLogger = std::make_shared<UMPS::Logging::StandardOut> (); 
        }
        else
        {
            mLogger = logger;
        }
        mIncrementerReplier = std::make_unique<Replier> (mContext, mLogger);
        mCounter = std::make_shared<Counter> ();
    }
    /// Stops the proxy and authenticator and joins threads
    void stop()
    {
        mLogger->debug("Incrementer stopping threads...");
        setRunning(false);
        if (mIncrementerReplier != nullptr)
        {
            if (mIncrementerReplier->isRunning()){mIncrementerReplier->stop();}
        }
    }
    /// Starts the proxy and authenticator and creates threads
    void start()
    {
        stop();
        setRunning(true);
        std::lock_guard<std::mutex> lockGuard(mMutex);
#ifndef NDEBUG
        assert(mIncrementerReplier->isInitialized());
#endif
        mLogger->debug("Starting the replier...");
        mIncrementerReplier->start();
#ifndef NDEBUG
        assert(mIncrementerReplier->isRunning());
#endif
    }
    /// @result True indicates the threads should keep running
    [[nodiscard]] bool keepRunning() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mKeepRunning;
    }
    /// @brief Toggles this as running or not running
    void setRunning(const bool running)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = running;
    }
    /// Destructor
    ~ServiceImpl()
    {
        stop();
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<Replier> mIncrementerReplier{nullptr};
    std::shared_ptr<Counter> mCounter{nullptr};
    Options mOptions;
    std::string mName = "Incrementer";
    bool mKeepRunning = true;
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

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ServiceImpl> (context, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (context, logger))
{
}

/// Destructor
Service::~Service() = default;

/// Is the service running
bool Service::isRunning() const noexcept
{
    return pImpl->keepRunning();
}

/// Initialize
void Service::initialize(const Options &options)
{
    stop(); // Ensure the service is stopped
    if (!options.haveBackendAddress())
    {
        throw std::runtime_error("Backend address not set");
    }
    // Counter
    auto sqlite3File = options.getSqlite3FileName();
    auto deleteIfExists = options.deleteSqlite3FileIfExists();
    pImpl->mCounter->initialize(sqlite3File, deleteIfExists);
    int64_t initialValue = options.getInitialValue();
    int increment = options.getIncrement();
    std::set<std::string> defaultItems{"Amplitude",
                                       "Event",
                                       "Magnitude",
                                       "Origin",
                                       "PhasePick",
                                       "PhaseArrival"};
    for (const auto &item : defaultItems)
    {
        if (!pImpl->mCounter->haveItem(item))
        {
            pImpl->mCounter->addItem(item, initialValue, increment);
        }
    }
    // Replier options
    ReplierOptions replierOptions;
    replierOptions.setAddress(options.getBackendAddress());
    replierOptions.setZAPOptions(options.getZAPOptions());
    pImpl->mIncrementerReplier->initialize(replierOptions,
                                           pImpl->mCounter);
    pImpl->mInitialized = pImpl->mIncrementerReplier->isInitialized();
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

/*
/// Gets the service name
std::string Service::getName() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mName;
}
*/

/// Stop the service
void Service::stop()
{
    pImpl->stop();
}

