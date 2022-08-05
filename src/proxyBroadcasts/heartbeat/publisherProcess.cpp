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
#include "umps/authentication/zapOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/logging/stdout.hpp"

using namespace UMPS::ProxyBroadcasts::Heartbeat;
namespace UCI = UMPS::Services::ConnectionInformation;

class PublisherProcess::PublisherProcessImpl
{
public:
    explicit PublisherProcessImpl(std::shared_ptr<UMPS::Logging::ILog> &logger):
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// Destructor
    ~PublisherProcessImpl()
    {
        stop();
    }
    /// Start the module
    void start()
    {
        stop(); // Make sure everything stopped
        // Now start it up
    }
    /// Stop the module
    void stop()
    {
        setRunning(false);
        if (mPublisherThread.joinable()){mPublisherThread.join();}
    }
    /// Initialized
    void setInitialized(const bool initialized)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mInitialized = initialized;
    }
    /// Initialized?
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
    std::thread mPublisherThread;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::unique_ptr<Publisher> mPublisher{nullptr};
    PublisherProcessOptions mOptions;
    bool mKeepRunning{false};
    bool mInitialized{false};
};

/// C'tor
PublisherProcess::PublisherProcess(
    std::shared_ptr<UMPS::Logging::ILog> logger) :
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
}

// Create a heartbeat publisher process from the ini file
std::unique_ptr<UMPS::ProxyBroadcasts::Heartbeat::PublisherProcess>
    UMPS::ProxyBroadcasts::Heartbeat::createHeartbeatProcess(
        std::shared_ptr<UCI::Requestor> &requestorIn,
        const std::string &iniFile,
        const std::string &section,
        std::shared_ptr<UMPS::Messaging::Context> context,
        std::shared_ptr<UMPS::Logging::ILog> logger)
{
    // Do I need to make the operator requestor?
    std::shared_ptr<UCI::Requestor> requestor{requestorIn};
    bool makeRequestor{false};
    if (requestorIn == nullptr)
    { 
        makeRequestor = true;
    }
    else if (!requestorIn->isInitialized())
    {
        makeRequestor = true;
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
        auto iInterval = static_cast<int> (interval.count());
        iInterval = propertyTree.get<int> (section + ".interval", iInterval);
        interval = std::chrono::seconds {iInterval};
        processOptions.setInterval(interval);
    
        // Yes, I need to make the requestor
        if (makeRequestor)
        {
            if (!std::filesystem::exists(iniFile))
            {
                throw std::invalid_argument("Initialization file: " + iniFile
                                         + " does not exist");
            }
            // I actually can save this...
            const std::string operatorSection{"uOperator"};
            std::string operatorAddress
                = propertyTree.get<std::string> (operatorSection + ".address",
                                                 "");
            if (operatorAddress.empty())
            {
                throw std::runtime_error(operatorSection
                                       + ".address not not defined");
            }
       
            auto zapOperatorOptions
               = UMPS::Modules::Operator::readZAPClientOptions(iniFile);
            UCI::RequestorOptions operatorOptions;
            operatorOptions.setAddress(operatorAddress);
            operatorOptions.setZAPOptions(zapOperatorOptions);

            requestor = std::make_shared<UCI::Requestor> (logger);
            requestor->initialize(operatorOptions);
        }
    } // End check on ini file
    // Get the heartbeat broadcast's address and the ZAP options
    auto address
        = requestor->getProxyBroadcastFrontendDetails(broadcast).getAddress();
    auto zapOptions = requestor->getZAPOptions();
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