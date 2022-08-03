#include <string>
#include <map>
#include <filesystem>
#include <mutex>
#include <thread>
#include <chrono>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/modules/module.hpp"
#include "umps/modules/process.hpp"
#include "umps/proxyBroadcasts/heartbeat/status.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisher.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcess.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/logging/level.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/messaging/context.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"

namespace UHeartbeat = UMPS::ProxyBroadcasts::Heartbeat;
namespace UAuth = UMPS::Authentication;
namespace UCI = UMPS::Services::ConnectionInformation;
using namespace UMPS::Modules;

/*
namespace
{
std::unique_ptr<UMPS::ProxyBroadcasts::Heartbeat::PublisherProcess>
    createHeartbeatProcess(std::shared_ptr<UCI::Requestor> &requestor,
                           const UAuth::ZAPOptions &zapOptions,
                           const std::chrono::seconds &interval = std::chrono::seconds {30},
                           const std::string &broadcast = "Heartbeat",
                           std::shared_ptr<zmq::context_t> context = nullptr,
                           std::shared_ptr<UMPS::Logging::ILog> logger = nullptr)
{
    namespace UHeartbeat = UMPS::ProxyBroadcasts::Heartbeat;
    // Get the heartbeat broadcast's address
    auto address
        = requestor->getProxyBroadcastFrontendDetails(broadcast).getAddress();
    // Create the publisher connection
    UHeartbeat::PublisherOptions connectionOptions;
    connectionOptions.setAddress(address);
    connectionOptions.setZAPOptions(zapOptions);
    auto connection = std::make_unique<UHeartbeat::Publisher> (context, logger);
    connection->initialize(connectionOptions);
#ifndef NDEBUG
    assert(connection->isInitialized());
#endif
    // Create the publisher options
    UHeartbeat::PublisherProcessOptions processOptions;
    processOptions.setInterval(interval); 
    // Create the publisher
    auto process = std::make_unique<UHeartbeat::PublisherProcess> ();
    process->initialize(processOptions, std::move(connection));
    return process;
}
}
*/

class IModule::IModuleImpl
{
public:
    IModuleImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mContext(std::make_shared<UMPS::Messaging::Context> (1))
    {
    }
    ~IModuleImpl()
    {
        stop();
    }
    // Create the UCI requestor
    void createConnectionInformationRequestor()
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        if (mConnectionInformationRequestor != nullptr)
        {
            mLogger->debug("Already connected to uOperator");
            return;
        }
        mLogger->debug("Connecting to uOperator at: " + mOperatorAddress);

        UCI::RequestorOptions options;
        options.setAddress(mOperatorAddress);
        options.setZAPOptions(mZAPOptions);

        mConnectionInformationRequestor
            = std::make_shared<UCI::Requestor> (mContext, mLogger);
        mConnectionInformationRequestor->initialize(options);
    }
    // Connect
    void connect()
    {
        disconnect();
        // Ensure the connection information requestor is created
        createConnectionInformationRequestor();
        std::lock_guard<std::mutex> lockGuard(mMutex);
        // Create the heartbeat publisher
        mLogger->debug("Connecting to heartbeat broadcast...");
        UHeartbeat::PublisherOptions heartbeatOptions;
        auto heartbeatAddress
            = mConnectionInformationRequestor->getProxyBroadcastFrontendDetails(
                 mHeartbeatBroadcastName).getAddress();
        heartbeatOptions.setAddress(heartbeatAddress);
        heartbeatOptions.setZAPOptions(mZAPOptions);
        mHeartbeatPublisher
            = std::make_shared<UHeartbeat::Publisher> (mContext, mLogger);
        mHeartbeatPublisher->initialize(heartbeatOptions);
        // Create the command

        mLogger->debug("Connected to heartbeat and command!");
        mConnected = true;
        mKeepRunning = true;
    }
    // Disconnect
    void disconnect()
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        if (mConnectionInformationRequestor != nullptr)
        {
            mLogger->debug("Disconnecting from uOperator");
            mConnectionInformationRequestor->disconnect();
            mConnectionInformationRequestor = nullptr;
        }
        if (mHeartbeatPublisher != nullptr)
        {
            mLogger->debug("Disconnecting from heartbeat broadcast");
            mHeartbeatPublisher->disconnect();
            mHeartbeatPublisher = nullptr;
        }
        mConnected = false;
    }
    /// @brief Toggles this as running or not running
    void setRunning(const bool running)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = running;
    }
    /// Keep running?
    bool keepRunning() const noexcept
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mKeepRunning;
    }
    /// Stop threads 
    void stop()
    {
        setRunning(false);
        for (auto &process : mProcesses)
        {
            process.second->stop();
        }
        mLogger->debug("IModule stopping threads...");
    }
    /// Start threads
    void start()
    {
        stop();
        setRunning(true);
        for (auto &process : mProcesses)
        {
            process.second->start();
        }
        mLogger->debug("IModule starting threads...");
    } 
    /// Add a process
    void addProcess(std::unique_ptr<IProcess> &process)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        auto name = process->getName();
        if (mProcesses.contains(name))
        {
            throw std::invalid_argument("Process: " + name + " already exists");
        }
        mProcesses.insert(std::pair {name, std::move(process)});
    }
    mutable std::mutex mMutex;
    std::map<std::string, std::unique_ptr<IProcess>> mProcesses;
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UCI::Requestor> mConnectionInformationRequestor{nullptr};
    std::shared_ptr<UHeartbeat::Publisher> mHeartbeatPublisher{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    UAuth::ZAPOptions mZAPOptions;
    std::string mOperatorAddress;
    std::string mHeartbeatBroadcastName{"Heartbeat"};
    std::string mLogFileDirectory{"/var/log/umps"};
    std::string mModuleName{"IModule"};
    std::chrono::seconds mHeartbeatInterval{30};
    UMPS::Logging::Level mVerbosity{UMPS::Logging::Level::INFO};
    bool mConnected{false};
    bool mKeepRunning{true};
};

/// C'tor
IModule::IModule() :
    pImpl(std::make_unique<IModuleImpl> ())
{
}

/// Destructor
IModule::~IModule() = default;

/// Initialization file
void IModule::parseInitializationFile(const std::string &initializationFile)
{
    if (!std::filesystem::exists(initializationFile))
    {
        throw std::invalid_argument("Initialization file: " + initializationFile
                                  + " does not exist");
    }
    // General module information:
    std::string generalSection = "General";
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(initializationFile,
                                               propertyTree);
    // Module name
    auto moduleName
        = propertyTree.get<std::string> (generalSection + ".moduleName",
                                         getName());
    setName(moduleName);     
    // Verbosity
    auto verbosity = static_cast<UMPS::Logging::Level>
                     (propertyTree.get<int> (generalSection + ".verbose",
                                            static_cast<int> (getVerbosity())));
    setVerbosity(verbosity);
    // Log file directory
    setLogFileDirectory(propertyTree.get<std::string>
                        (generalSection + ".logFileDirectory",
                         getLogFileDirectory()));

    // Essential uOperator connection information:
    // Need the operator's IP address
    const std::string operatorSection = "uOperator";
    std::string address
        = propertyTree.get<std::string> (operatorSection + ".address", "");
    if (address.empty())
    {
        throw std::runtime_error(operatorSection + ".address not not defined");
    }
    setOperatorAddress(address);
    auto zapOptions
        = UMPS::Modules::Operator::readZAPClientOptions(initializationFile);
    setZAPOptions(zapOptions);
    // Connect to operator
    pImpl->createConnectionInformationRequestor();

    // Other broadcasts/command queues:
    // Heartbeat broadcast name
    ProxyBroadcasts::Heartbeat::PublisherProcessOptions heartbeatPublisherOptions;

    const std::string heartbeatSection = "Heartbeat";
    std::string heartbeatBroadcastName{"Heartbeat"};
    heartbeatBroadcastName
        = propertyTree.get<std::string> (heartbeatSection + ".name",
                                         heartbeatBroadcastName);
 
    auto heartbeatInterval = static_cast<int> (heartbeatPublisherOptions.getInterval().count());
    heartbeatInterval = propertyTree.get<int> (heartbeatSection + ".interval",
                                               heartbeatInterval);
    setHeartbeatInterval(std::chrono::seconds{heartbeatInterval});

/*
    createHeartbeatProcess(pImpl->mConnectionInformationRequestor,
                           zapOptions,
                           std::chrono::seconds {heartbeatInterval},
                           heartbeatBroadcastName,
                           pImpl->mContext,
                           pImpl->mLogger);
*/
/*
    heartbeatPublisherOptions.setInterval(std::chrono::seconds{heartbeatInterval});
    heartbeatPublisherOptions.setBroadcastName(heartbeatBroadcastName);
*/    
}

/// ZAP options
void IModule::setZAPOptions(const UAuth::ZAPOptions &options) noexcept
{
    pImpl->mZAPOptions = options;
}
 
UAuth::ZAPOptions IModule::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// UOperator address
void IModule::setOperatorAddress(const std::string &address)
{
    if (address.empty()){throw std::runtime_error("Address is empty");}
    pImpl->mOperatorAddress = address;
}

std::string IModule::getOperatorAddress() const
{
    if (!haveOperatorAddress())
    {
        throw std::runtime_error("Operator address not set");
    }
    return pImpl->mOperatorAddress;
}

bool IModule::haveOperatorAddress() const noexcept
{
    return !pImpl->mOperatorAddress.empty();
}

/// Heartbeat name
void IModule::setHeartbeatBroadcastName(const std::string &name)
{
    if (name.empty())
    {
        throw std::runtime_error("Heartbeat broadcast name: " + name
                               + " is empty");
    }
    pImpl->mHeartbeatBroadcastName = name;
}

std::string IModule::getHeartbeatBroadcastName() const noexcept
{
    return pImpl->mHeartbeatBroadcastName;
}

/// Heartbeat interval
void IModule::setHeartbeatInterval(const std::chrono::seconds &interval)
{
    if (interval.count() <= 0)
    {
        throw std::invalid_argument("Heartbeat interval must be positive");
    }
    pImpl->mHeartbeatInterval = interval;
}

std::chrono::seconds IModule::getHeartbeatInterval() const noexcept
{
    return pImpl->mHeartbeatInterval;
}

/// Modules verbosity
void IModule::setVerbosity(const UMPS::Logging::Level verbosity) noexcept
{
    pImpl->mVerbosity = verbosity;
}

UMPS::Logging::Level IModule::getVerbosity() const noexcept
{
    return pImpl->mVerbosity;
}

/// Logging file directory
void IModule::setLogFileDirectory(const std::string &logFileDirectory)
{
    if (logFileDirectory.empty() || logFileDirectory == ".")
    {
        pImpl->mLogFileDirectory = "./";
        return;
    }
    // Ensure the directory exists
    if (!std::filesystem::exists(logFileDirectory))
    {
        if (!std::filesystem::create_directories(logFileDirectory))
        {
            throw std::runtime_error("Failed to create log file directory: "
                                   + logFileDirectory);
        }
    }
    pImpl->mLogFileDirectory = logFileDirectory; 
}

std::string IModule::getLogFileDirectory() const noexcept
{
    return pImpl->mLogFileDirectory;
}

/// The application's logger
void IModule::setLogger(std::shared_ptr<UMPS::Logging::ILog> &logger)
{
    if (logger == nullptr)
    {
        throw std::invalid_argument("Logger cannot be NULL");
    }
    pImpl->mLogger = logger;
}

std::shared_ptr<UMPS::Logging::ILog> IModule::getLogger() const noexcept
{
    return pImpl->mLogger;
}

/// Get the connection information properties
std::shared_ptr<UCI::Requestor>
    IModule::getConnectionInformationRequestor() const
{
    if (!haveOperatorAddress())
    {
        throw std::runtime_error("Operator address not set");
    }
    pImpl->createConnectionInformationRequestor();
    return pImpl->mConnectionInformationRequestor;
}

/// The module's name
void IModule::setName(const std::string &name)
{
    if (name.empty())
    {
        throw std::invalid_argument("Module name cannot be empty");
    }
    pImpl->mModuleName = name;
}

std::string IModule::getName() const noexcept
{
    return pImpl->mModuleName;
}

/// Connect
void IModule::connect()
{
    if (!haveOperatorAddress())
    {
        throw std::runtime_error("Operator address not set");
    }
    if (isConnected()){disconnect();}
    pImpl->connect();
}

/// Disconnect
void IModule::disconnect()
{
    pImpl->disconnect();
}

/// Connected?
bool IModule::isConnected() const noexcept
{
    return pImpl->mConnected;
}

/// Keep running?
bool IModule::keepRunning() const noexcept
{
    return pImpl->keepRunning();
}

/// Stop
void IModule::stop()
{
    pImpl->stop();    
}

void IModule::start()
{
    pImpl->start();
}
