#include <mutex>
#include <thread>
#include <string>
#ifndef NDEBUG
#include <cassert>
#endif
#include <filesystem>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <zmq.hpp>
#include "umps/proxyBroadcasts/heartbeat/publisherProcess.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisher.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/status.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/logging/stdout.hpp"
#include "private/threadSafeQueue.hpp"

using namespace UMPS::ProxyBroadcasts::Heartbeat;
namespace UCI = UMPS::Services::ConnectionInformation;

class PublisherProcess::PublisherProcessImpl
{
public:
    /// @brief C'tor.
    explicit PublisherProcessImpl(
        const std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// @brief Destructor
    ~PublisherProcessImpl()
    {
        stop();
    }
    /// @brief Sets the status message to publish
    void setStatus(const Status &status)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mStatus = status;
    }
    /// @brief Sends a status message
    void sendStatus(const Status &status)
    {
        mStatusQueue.push(status);
    }
    /// @brief Actually publish the message
    void publishStatus()
    {
        Status status;
        mLogger->debug("Heartbeat publisher status starting...");
        try
        {
            Status startStatus;
            startStatus.setModule(mModule);
            startStatus.setModuleStatus(ModuleStatus::Alive);
            mPublisher->send(startStatus);
        }
        catch (const std::exception &e)
        {
            mLogger->error("Failed to send start status:  Failed with:\n"
                         + std::string{e.what()});
        }
        while (keepRunning())
        {
            if (mStatusQueue.wait_until_and_pop(&status))
            {
                try
                {
                    mPublisher->send(status);
                }
                catch (const std::exception &e) 
                {
                    mLogger->error("Failed to send status:  Failed with:\n"
                                 + std::string{e.what()});
                }
            }
        }
        // I'll make and send the last message.  This prevents a weird edge
        // case on shut down wehre this thread exits and sendStatus pushes
        // an exit message but no one is there to send it.
        try
        {
            Status exitStatus;
            exitStatus.setModule(mModule);
            exitStatus.setModuleStatus(ModuleStatus::Disconnected);
            mPublisher->send(exitStatus);
        }
        catch (const std::exception &e)
        {
            mLogger->error("Failed to send exit status:  Failed with\n"
                         + std::string{e.what()});
        }
        mLogger->debug("Heartbeat publisher thread exiting...");
    }
    /// @brief Just keep cranking out a message that the module is running.
    void sendMyStatus()
    {
        mLogger->debug("Heartbeat okay status thread starting...");
        // I'm forcing this to be something large - seconds
        auto interval = mOptions.getInterval();
        // To make this seem responsive wake up every 0.1 seconds
        constexpr std::chrono::milliseconds fineInterval{500};
        std::chrono::milliseconds waitTime{0};
        while (keepRunning())
        {
            // I should continue to wait.  But not too long incase someone
            // terminates the program.
            if (waitTime < interval)
            {
                std::this_thread::sleep_for(fineInterval);
                waitTime = waitTime + fineInterval;
            }
            else // My turn to do something
            {
                mStatus.setTimeStampToNow();
                mStatusQueue.push(mStatus);
                waitTime = std::chrono::milliseconds {0}; // Reset
            }
        }
        mLogger->debug("Heartbeat okay status thread exiting...");
    }
    /// @brief Start the heartbeat process.
    void start()
    {
        stop(); // Make sure everything stopped
        // Indicate this should be running
        setRunning(true);
        // Return to default status message
        mStatus.setModule(mModule);
        mStatus.setModuleStatus(ModuleStatus::Alive);
        mStatus.setTimeStampToNow();
        // Start threads
        mPublisherThread = std::thread(&PublisherProcessImpl::publishStatus,
                                       this);
        mStatusThread = std::thread(&PublisherProcessImpl::sendMyStatus,
                                    this);
    }
    /// @brief Stop the heartbeat process.
    void stop()
    {
        setRunning(false);
        if (mStatusThread.joinable()){mStatusThread.join();}
        if (mPublisherThread.joinable()){mPublisherThread.join();}
    }
    /// @brief Sets the class as being initialized or not.
    void setInitialized(const bool initialized)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mInitialized = initialized;
    }
    /// @result True indicates this class is initialized
    [[nodiscard]] bool isInitialized() const noexcept
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mInitialized;
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
///private:
    mutable std::mutex mMutex;
    ThreadSafeQueue<Status> mStatusQueue;
    Status mStatus;
    std::thread mPublisherThread;
    std::thread mStatusThread;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<Publisher> mPublisher{nullptr};
    PublisherProcessOptions mOptions;
    std::string mModule{"Unknown"};
    bool mKeepRunning{false};
    bool mInitialized{false};
};

/// C'tor
PublisherProcess::PublisherProcess() :
    pImpl(std::make_unique<PublisherProcessImpl> (nullptr))
{
}

PublisherProcess::PublisherProcess(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherProcessImpl> (logger))
{
}

/// Destructor
PublisherProcess::~PublisherProcess() = default;

/// Intiialized
bool PublisherProcess::isInitialized() const noexcept
{
    return pImpl->isInitialized();
}

/// Start
void PublisherProcess::start()
{
    pImpl->start();
}

/// Stop 
void PublisherProcess::stop()
{
    pImpl->stop();
}

/// Running?
bool PublisherProcess::isRunning() const noexcept
{
    return pImpl->keepRunning();
}

/// Initialize the class
void PublisherProcess::initialize(const PublisherProcessOptions &options,
                                  std::unique_ptr<Publisher> &&publisher)
{
    if (!publisher->isInitialized())
    {
        throw std::invalid_argument("Publisher not initialized");
    }
    stop(); // If this exists then stop it
    // Okay, now make it
    pImpl->mPublisher = std::move(publisher);
    pImpl->mOptions = options;
    pImpl->setInitialized(true);
}

/// Set the module status
void PublisherProcess::setStatus(const Status &status)
{
    if (!isRunning())
    {
        throw std::runtime_error("Heartbeat process not running");
    }
    pImpl->setStatus(status);
}

void PublisherProcess::sendStatus(const Status &status) const
{
    if (!isRunning())
    {
        throw std::runtime_error("Heartbeat process not running");
    }
    pImpl->sendStatus(status);
}

/// Name of module
std::string PublisherProcess::getName() const noexcept
{
    return pImpl->mOptions.getName();
}

// Create a heartbeat publisher process from the ini file
std::unique_ptr<UMPS::ProxyBroadcasts::Heartbeat::PublisherProcess>
    UMPS::ProxyBroadcasts::Heartbeat::createHeartbeatProcess(
        const UCI::Requestor &requestor,
        const std::string &iniFile,
        const std::string &section,
        std::shared_ptr<UMPS::Messaging::Context> context,
        std::shared_ptr<UMPS::Logging::ILog> logger)
{
    // Do I need to make the operator requestor?
    if (!requestor.isInitialized())
    {
        throw std::invalid_argument("uOperator requestor not initialized");
    }
    // Set the defaults
    PublisherProcessOptions processOptions;
    std::string broadcast = "Heartbeat";
    auto interval = processOptions.getInterval();
    // Load things from the initialization file if possible 
    boost::property_tree::ptree propertyTree;
    if (std::filesystem::exists(iniFile))
    {
        boost::property_tree::ini_parser::read_ini(iniFile,
                                                   propertyTree);
        // Get broadcast name
        broadcast = propertyTree.get<std::string> (section + ".broadcast",
                                                   broadcast);
        if (broadcast.empty())
        {
            throw std::runtime_error("Heartbeat broadcast not set");
        }
        // Interval
        auto iInterval = static_cast<int> (interval.count());
        iInterval = propertyTree.get<int> (section + ".interval", iInterval);
        interval = std::chrono::seconds {iInterval};
        processOptions.setInterval(interval);
        // Custom process name
        auto processName
            = propertyTree.get<std::string> (section + ".processName",
                                             processOptions.getName());
        processOptions.setName(processName);
    } // End check on ini file
    // Get the heartbeat broadcast's address and the ZAP options
    auto address
        = requestor.getProxyBroadcastFrontendDetails(broadcast).getAddress();
    auto zapOptions = requestor.getZAPOptions();
    // Create the publisher
    PublisherOptions publisherOptions;
    publisherOptions.setAddress(address);
    publisherOptions.setZAPOptions(zapOptions);

    auto publisher = std::make_unique<Publisher> (context, logger);
    publisher->initialize(publisherOptions);

    // Create the publisher process
    auto process = std::make_unique<PublisherProcess> (logger);
    process->initialize(processOptions, std::move(publisher));
#ifndef NDEBUG
    assert(process->isInitialized());
#endif
    return process;
}
