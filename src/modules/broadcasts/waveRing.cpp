#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#ifndef NDEBUG
#include <cassert>
#endif
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/modules/module.hpp"
#include "umps/modules/process.hpp"
#include "umps/modules/processManager.hpp"
#include "umps/logging/spdlog.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/earthworm/traceBuf2.hpp"
#include "umps/earthworm/waveRing.hpp"
#include "umps/proxyBroadcasts/dataPacket/publisher.hpp"
#include "umps/proxyBroadcasts/dataPacket/publisherOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcess.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/localService.hpp"
#include "umps/services/command/localServiceOptions.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "private/isEmpty.hpp"

#include "umps/proxyBroadcasts/dataPacket/subscriber.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
 
#define MODULE_NAME "broadcastWaveRing"

namespace UAuth = UMPS::Authentication;
namespace UServices = UMPS::Services;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UModules = UMPS::Modules;
namespace UHeartbeat = UMPS::ProxyBroadcasts::Heartbeat;

namespace
{

/// @result Gets the command line input options as a string.
[[nodiscard]] std::string getInputOptions() noexcept
{
    std::string commands;
    commands = "Commands:\n";
    commands = commands + "   quit   Exits the program.\n";
    commands = commands + "   help   Displays this message.\n";
    return commands;
}

/// @resultGets the input line.
[[nodiscard]] std::string getInputLine() noexcept
{
    return std::string{MODULE_NAME} + "$";
}

}

/// @brief Defines the module options.
struct ProgramOptions
{
    /// @brief Load the module options from an initialization file.
    void parseInitializationFile(const std::string &iniFile,
                                 const std::string generalSection = "General")
    {
        boost::property_tree::ptree propertyTree;
        boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
        //------------------------------ General -----------------------------//
        // Module name
        mModuleName
            = propertyTree.get<std::string> (generalSection + ".moduleName",
                                             mModuleName);
        if (mModuleName.empty())
        {
            throw std::runtime_error("Module name not defined");
        }
        // Verbosity
        mVerbosity = static_cast<UMPS::Logging::Level>
                     (propertyTree.get<int> (generalSection + ".verbose",
                                             static_cast<int> (mVerbosity)));
        // Log file directory
        mLogFileDirectory = propertyTree.get<std::string>
            (generalSection + ".logFileDirectory",
             mLogFileDirectory.string());
        if (!mLogFileDirectory.empty() &&
            !std::filesystem::exists(mLogFileDirectory))
        {
            std::cout << "Creating log file directory: "
                      << mLogFileDirectory << std::endl;
            if (!std::filesystem::create_directories(mLogFileDirectory))
            {
                throw std::runtime_error("Failed to make log directory");
            }
        }
        // Broadcast name
        mDataPacketBroadcastName
            = propertyTree.get<std::string>
                ("General.dataPacketBroadcast", mDataPacketBroadcastName);
        if (mDataPacketBroadcastName.empty())
        {
            throw std::runtime_error("General.dataPacketBroadcast not set");
        }
        //----------------------------- Earthworm ----------------------------//
        // EW_PARAMS environment variable
        mEarthwormParametersDirectory = propertyTree.get<std::string>
                                        ("Earthworm.ewParams",
                                         mEarthwormParametersDirectory);
        if (!std::filesystem::exists(mEarthwormParametersDirectory))
        {
            throw std::runtime_error("Earthworm parameters directory: "
                                   + mEarthwormParametersDirectory
                                   + " does not exist");
        }
        // EW_INST environment variable
        mEarthwormInstallation = propertyTree.get<std::string>
                                 ("Earthworm.ewInstallation",
                                  mEarthwormInstallation);
        // Earthworm wave ring
        mEarthwormWaveRingName = propertyTree.get<std::string>
                                 ("Earthworm.waveRingName",
                                  mEarthwormWaveRingName);
        // Wait after reading
        mEarthwormWait = propertyTree.get<int> ("Earthworm.wait",
                                                mEarthwormWait);
        if (mEarthwormWait < 0)
        {
            std::cerr << "Setting wait time to 0" << std::endl;
            mEarthwormWait = 0;
        }
    }
    UAuth::ZAPOptions mZAPOptions;
    std::string mEarthwormParametersDirectory
        {"/opt/earthworm/run_working/params/"};
    std::string mModuleName{MODULE_NAME};
    std::string mEarthwormInstallation{"INST_UNKNOWN"};
    std::string mEarthwormWaveRingName{"WAVE_RING"};
    //std::string operatorAddress;
    std::string mDataPacketBroadcastName{"DataPacket"};
    std::string mHeartbeatBroadcastName{"Heartbeat"};
    std::filesystem::path mLogFileDirectory{"/var/log/umps"};
    std::chrono::seconds heartBeatInterval{30};
    int mEarthwormWait{0};
    UMPS::Logging::Level mVerbosity = UMPS::Logging::Level::Info;
};

/// @result The logger for this application.
std::shared_ptr<UMPS::Logging::ILog>
    createLogger(const std::string &moduleName,
                 const std::filesystem::path logFileDirectory = "/var/log/umps",
                 const UMPS::Logging::Level verbosity = UMPS::Logging::Level::Info,
                 const int hour = 0, const int minute = 0)
{
    auto logFileName = moduleName + ".log";  
    auto fullLogFileName = logFileDirectory / logFileName;
    auto logger = std::make_shared<UMPS::Logging::SpdLog> ();
    logger->initialize(moduleName,
                       fullLogFileName,
                       verbosity,
                       hour, minute);
    logger->info("Starting logging for " + moduleName);
    return logger;
}

/// @brief Create utility to attach to and read from earthworm wave ring
std::unique_ptr<UMPS::Earthworm::WaveRing> 
    createEarthwormWaveRingReader(const ProgramOptions &options,
                                  std::shared_ptr<UMPS::Logging::ILog> &logger,
                                  const bool flushRing = true)
{
    logger->debug("Attaching to earthworm ring: "
                + options.mEarthwormWaveRingName);
    setenv("EW_PARAMS", options.mEarthwormParametersDirectory.c_str(), true);
    setenv("EW_INSTALLATION", options.mEarthwormInstallation.c_str(), true);
    auto earthwormWaveRing
        = std::make_unique<UMPS::Earthworm::WaveRing> (logger);
    earthwormWaveRing->connect(options.mEarthwormWaveRingName,
                               options.mEarthwormWait);
    if (flushRing){earthwormWaveRing->flush();}
    logger->debug("Attach to earthworm ring: "
                + options.mEarthwormWaveRingName);
    return earthwormWaveRing;
}


//----------------------------------------------------------------------------//
/// @brief Reads packets from the wave ring then brodcasts them to UMPS.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class BroadcastPackets : public UMPS::Modules::IProcess
{
public:
    BroadcastPackets() = default;
    BroadcastPackets(
        const std::string &moduleName, 
        std::unique_ptr<UMPS::ProxyBroadcasts::DataPacket::Publisher>
             &&packetPublisher,
        std::unique_ptr<UMPS::Earthworm::WaveRing> &&waveRing,
        std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mPacketPublisher(std::move(packetPublisher)),
        mWaveRing(std::move(waveRing)),
        mLogger(logger)
    {
        if (mWaveRing == nullptr)
        {
            throw std::invalid_argument("Wave ring is NULL");
        }
        if (mPacketPublisher == nullptr)
        {
            throw std::invalid_argument("Packet publisher is NULL");
        }
        if (!mWaveRing->isConnected())
        {
            throw std::invalid_argument("Wave ring reader not connected");
        }
        if (!mPacketPublisher->isInitialized())
        {
            throw std::invalid_argument("Packet publisher not initialized");
        }
        if (mLogger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        mLocalCommand
            = std::make_unique<UMPS::Services::Command::LocalService> (mLogger);
        UMPS::Services::Command::LocalServiceOptions localServiceOptions;
        localServiceOptions.setModuleName(moduleName);
        localServiceOptions.setCallback(
            std::bind(&BroadcastPackets::commandCallback,
                      this,
                      std::placeholders::_1,
                      std::placeholders::_2,
                      std::placeholders::_3));
        mLocalCommand->initialize(localServiceOptions);
        mInitialized = true;
    }
    /// Initialized?
    [[nodiscard]] bool isInitialized() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mInitialized;
    }
    /// Destructor
    ~BroadcastPackets() override
    {
        stop();
    }
    [[nodiscard]] bool keepRunning() const
    {
        std::scoped_lock lock(mMutex);
        return mKeepRunning; 
    }
    /// @brief Toggles this as running or not running
    void setRunning(const bool running)
    {   
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = running;
    }
    /// @brief Gets the process name.
    std::string getName() const noexcept override
    {
        return "BroadcastPackets";
    }
    /// @brief Stops the process.
    void stop() override
    {
        setRunning(false);
        if (mBroadcastThread.joinable()){mBroadcastThread.join();}
        if (mLocalCommand->isRunning()){mLocalCommand->stop();}
    }
    /// @brief Starts the process.
    void start() override
    {
        stop();
        if (!isInitialized())
        {
            throw std::runtime_error("Class not initialized");
        }
        setRunning(true);
        mLogger->debug("Starting the wave ring broadcast thread...");
        mBroadcastThread = std::thread(&BroadcastPackets::run,
                                       this);
        mLogger->debug("Starting the local command proxy..."); 
        mLocalCommand->start();
    }
    /// Running?
    [[nodiscard]] bool isRunning() const noexcept override
    {
        return keepRunning();
    }
    void run()
    {
        if (!mWaveRing->isConnected())
        {
            throw std::runtime_error("Wave ring not yet connected");
        }
        if (!mPacketPublisher->isInitialized())
        { 
            throw std::runtime_error("Publisher not yet initialized");
        }
        mWaveRing->flush();
        mLogger->debug("Earthworm broadcast thread is starting");
        while (keepRunning())
        {
            auto startClock = std::chrono::high_resolution_clock::now();
            // Read from the earthworm ring
            try
            {
                mWaveRing->read();
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed reading wave ring.  Failed with:\n"
                             + std::string(e.what()));
                continue;
            }
            auto nMessages = mWaveRing->getNumberOfTraceBuf2Messages();
            auto traceBuf2MessagesPtr
                = mWaveRing->getTraceBuf2MessagesPointer();
            // Now broadcast the tracebufs as datapacket messages
            int nSent = 0;
            for (int iMessage = 0; iMessage < nMessages; ++iMessage)
            {
                // Send it
                try
                {
                    auto dataPacket
                        = traceBuf2MessagesPtr[iMessage].toDataPacket();
                    mPacketPublisher->send(dataPacket);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    nSent = nSent + 1;
                }
                catch (const std::exception &e)
                {
                    mLogger->error(e.what());
                }
            }
            auto endClock = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>
                            (endClock - startClock);
            if (duration < mBroadcastInterval)
            {
                auto wait = mBroadcastInterval - duration;
                std::this_thread::sleep_for(wait);
            }
        }
        mLogger->debug("Earthworm broadcast thread is terminating");
    }
    // Callback for local interaction
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        commandCallback(const std::string &messageType,
                        const void *data,
                        size_t length)
    {
        namespace USC = UMPS::Services::Command;
        mLogger->debug("Command request received"); 
        USC::AvailableCommandsRequest availableCommandsRequest;
        USC::CommandRequest commandRequest;
        if (messageType == availableCommandsRequest.getMessageType())
        {
            USC::AvailableCommandsResponse response;
            response.setCommands(getInputOptions());
            try
            {
                availableCommandsRequest.fromMessage(
                    static_cast<const char *> (data), length); 
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed to unpack commands request");
            }
            return response.clone(); 
        }
        else if (messageType == commandRequest.getMessageType())
        {
            USC::CommandResponse response;
            try
            {
                commandRequest.fromMessage( 
                    static_cast<const char *> (data), length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed to unpack text request");
                response.setReturnCode(
                    USC::CommandReturnCode::ApplicationError);
            }
            auto command = commandRequest.getCommand(); 
            if (command == "quit")
            {
                mLogger->debug("Issuing quit command...");
                response.setResponse("Bye!");
                response.setReturnCode(USC::CommandReturnCode::Success);
            }
            else
            {
                response.setResponse(getInputOptions());
                if (command != "help")
                {
                    mLogger->debug("Invalid command: " + command);
                    response.setResponse("Invalid command: " + command);
                    response.setReturnCode(USC::CommandReturnCode::InvalidCommand);
                }
                else
                {
                    response.setReturnCode(USC::CommandReturnCode::Success);
                }
            }
            return response.clone();
        }
        else
        {
            mLogger->error("Unhandled message type: " + messageType);
        }
        // Return
        mLogger->error("Unhandled message: " + messageType);
        USC::AvailableCommandsResponse commandsResponse; 
        commandsResponse.setCommands(getInputOptions());
        return commandsResponse.clone();
    }
    mutable std::mutex mMutex;
    std::thread mBroadcastThread;
    std::unique_ptr<UMPS::ProxyBroadcasts::DataPacket::Publisher>
         mPacketPublisher{nullptr};
    std::unique_ptr<UMPS::Earthworm::WaveRing> mWaveRing{nullptr};
    std::unique_ptr<UMPS::Services::Command::LocalService>
         mLocalCommand{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::chrono::seconds mBroadcastInterval{1};
    bool mKeepRunning{true};
    bool mInitialized{false};
};

/*
class CommandProcess : UMPS::Modules::IProcess
{
public:
    CommandProcess(std::shared_ptr<UMPS::Logging::ILog> logger = nullptr) :
        mLogger(logger)
    {    
        if (mLogger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// @brief Destructor
    ~CommandProcess()
    {
        stop();
    }
    /// @brief Gets the command line input options as a string.
    [[nodiscard]] std::string getInputOptions() const
    {
        std::string commands;
        commands = "Commands:\n";
        commands = commands + "   quit   Exits the program.\n";
        commands = commands + "   help   Displays this message.\n";
        return commands;
    }
    /// Running?
    [[nodiscard]] bool isRunning() const noexcept override
    {
        std::scoped_lock lock(mMutex);
        return mKeepRunning;
    }
    /// Start
    void start() override
    {
        stop();
        setRunning(true);
    }
    /// Stops a thread
    void stop() override
    {
        setRunning(false);
        if (mCommandThread.joinable()){mCommandThread.join();}
    }
    /// @brief Toggles this as running or not running
    void setRunning(const bool running)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = running;
    }
    /// @brief Gets the input line
    [[nodiscard]] static std::string getInputLine()
    {
        return std::string{MODULE_NAME} + "$";
    }
    /// @brief Handles the terminal interaction when running interactively.
    ///        This should be handled by the main thread.
    void processTerminal(const bool runInteractive = true)
    {
        while (isRunning())
        {
            if (runInteractive)
            {
                std::string command;
                std::cout << ::getInputLine();
                std::cin >> command;
                if (command == "quit")
                {
                    stop();
                }
                else
                {
                    std::cout << std::endl;
                    if (command != "help")
                    {
                        std::cout << "Unknown command: " << command << std::endl;
                        std::cout << std::endl;
                    }
                    std::cout << ::getInputOptions();
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        mLogger->debug("Terminal management thread exiting...");
    }
//private:
    mutable std::mutex mMutex;
    std::thread mCommandThread;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    bool mKeepRunning{true}; 
};
*/

/*
class BroadcastWaveRing
{
public:
    /// @brief The module name.
    [[nodiscard]] std::string getName() const noexcept
    {
        return mModuleName;
    }
    /// @brief Create the logger.
    void createLogger(const int hour = 0, const int minute = 0)
    {
        std::filesystem::path logFileDirectory{mLogFileDirectory};
        auto logFileName = getName() + ".log";  
        auto fullLogFileName = logFileDirectory / logFileName;
        UMPS::Logging::SpdLog logger;
        logger.initialize(getName(),
                          fullLogFileName,
                          mVerbosity,
                          hour, minute);
        mLogger = std::make_shared<UMPS::Logging::SpdLog> (logger);
    }
    /// @brief parse initialization file
    void parseInitializationFile(const std::string &initializationFile)
    {
        mInitializationFile = initializationFile;
        if (!std::filesystem::exists(mInitializationFile))
        {
            mInitializationFile = "";
            throw std::runtime_error("Initialization file: "
                                   + initializationFile + " does not exist");
        }
        // Now read everything else
        ProgramOptions options;
        boost::property_tree::ptree propertyTree;
        boost::property_tree::ini_parser::read_ini(initializationFile,
                                                   propertyTree);
                                                   
        mDataPacketBroadcastName
            = propertyTree.get<std::string>
                ("General.dataPacketBroadcast", mDataPacketBroadcastName);
        if (mDataPacketBroadcastName.empty())
        {
            throw std::runtime_error("General.dataPacketBroadcast not set");
        }
        //----------------------------- Earthworm ----------------------------//
        // EW_PARAMS environment variable
        mEarthwormParametersDirectory = propertyTree.get<std::string>
                                        ("Earthworm.ewParams",
                                         mEarthwormParametersDirectory);
        if (!std::filesystem::exists(mEarthwormParametersDirectory))
        {
            throw std::runtime_error("Earthworm parameters directory: "
                                   + mEarthwormParametersDirectory
                                   + " does not exist");
        }
        // EW_INST environment variable
        mEarthwormInstallation = propertyTree.get<std::string>
                                 ("Earthworm.ewInstallation",
                                  mEarthwormInstallation);
        // Earthworm wave ring    
        mEarthwormWaveRingName = propertyTree.get<std::string>
                                 ("Earthworm.waveRingName",
                                  mEarthwormWaveRingName);
        // Wait after reading     
        mEarthwormWait = propertyTree.get<int> ("Earthworm.wait",
                                                mEarthwormWait);
        if (mEarthwormWait < 0)                 
        {
            std::cerr << "Setting wait time to 0" << std::endl;
            mEarthwormWait = 0;
        }
    }
    /// @brief Create the heartbeat process
    void createHeartbeatProcess()
    {
        //createHeartbeatProcess( ); 
    }

    std::shared_ptr<zmq::context_t>
        mContext{std::make_shared<zmq::context_t> (1)};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::filesystem::path mInitializationFile;
    std::string mLogFileDirectory{"logs"};
    std::string mModuleName{"broadcastWaveRing"};
    std::shared_ptr<BroadcastPackets> mEarthwormBroadcast{nullptr};
    std::string mEarthwormParametersDirectory
        = "/opt/earthworm/run_working/params/";
    std::string mEarthwormInstallation = "INST_UNKNOWN";
    std::string mEarthwormWaveRingName = "WAVE_RING";
    std::string mDataPacketBroadcastName = "DataPacket";
    int mEarthwormWait{0};
    UMPS::Logging::Level mVerbosity{UMPS::Logging::Level::INFO};
};
*/

/*
class Module : public UMPS::Modules::IModule
{
public:
    /// Destructor
    ~Module() override
    {
        stop();
    }
    /// @brief parse initialization file
    void parseInitializationFile(const std::string &initializationFile) override
    {
        // Read the standard information
        IModule::parseInitializationFile(initializationFile);
        // Now read everything else
        ProgramOptions options;
        boost::property_tree::ptree propertyTree;
        boost::property_tree::ini_parser::read_ini(initializationFile,
                                                   propertyTree);

        mDataPacketBroadcastName
            = propertyTree.get<std::string>
                ("General.dataPacketBroadcast", mDataPacketBroadcastName);
        if (mDataPacketBroadcastName.empty())
        {
            throw std::runtime_error("General.dataPacketBroadcast not set");
        }
        //----------------------------- Earthworm ----------------------------//
        // EW_PARAMS environment variable
        mEarthwormParametersDirectory = propertyTree.get<std::string>
                                        ("Earthworm.ewParams",
                                         mEarthwormParametersDirectory);
        if (!std::filesystem::exists(mEarthwormParametersDirectory))
        {
            throw std::runtime_error("Earthworm parameters directory: "
                                   + mEarthwormParametersDirectory
                                   + " does not exist");
        }
        // EW_INST environment variable
        mEarthwormInstallation = propertyTree.get<std::string>
                                 ("Earthworm.ewInstallation",
                                  mEarthwormInstallation);
        // Earthworm wave ring
        mEarthwormWaveRingName = propertyTree.get<std::string>
                                 ("Earthworm.waveRingName",
                                  mEarthwormWaveRingName);
        // Wait after reading
        mEarthwormWait = propertyTree.get<int> ("Earthworm.wait",
                                                mEarthwormWait);
        if (mEarthwormWait < 0)
        {
            std::cerr << "Setting wait time to 0" << std::endl;
            mEarthwormWait = 0;
        }
    }
    /// Create the logger
    void createLogger(const int hour = 0, const int minute = 0)
    {
        std::filesystem::path logFileDirectory{IModule::getLogFileDirectory()};
        auto logFileName = getName() + ".log";  
        auto fullLogFileName = logFileDirectory / logFileName;
        UMPS::Logging::SpdLog logger;
        logger.initialize(getName(),
                          fullLogFileName,
                          IModule::getVerbosity(),
                          hour, minute);
        mLogger = std::make_shared<UMPS::Logging::SpdLog> (logger);
        IModule::setLogger(mLogger);
    }
    /// Connect to broadcasts
    void connect() override
    {
        // Create the core connections
        IModule::connect();
        //IModule::createHeartbeatBroadcast();
        // Get the data packet broadcast connection information
        auto uci = IModule::getConnectionInformationRequestor();
        auto packetAddress = uci->getProxyBroadcastFrontendDetails(
                               mDataPacketBroadcastName).getAddress();
 
        mLogger->debug("Connecting to data packet broadcast at: "
                     + packetAddress);
        UMPS::ProxyBroadcasts::DataPacket::PublisherOptions publisherOptions;
        publisherOptions.setAddress(packetAddress);
        publisherOptions.setZAPOptions(IModule::getZAPOptions());
        mPacketPublisher
            = std::make_shared<UMPS::ProxyBroadcasts::DataPacket::Publisher> (mLogger);
        mPacketPublisher->initialize(publisherOptions);
#ifndef NDEBUG
        assert(mPacketPublisher->isInitialized());
#endif
        mLogger->debug("Connected to data packet broadcast!");
    }
    /// @brief Attach to the earthworm ring
    void attachToEarthwormWaveRing(const bool flushRing = true)
    { 
        mLogger->debug("Attaching to earthworm ring: "
                     + mEarthwormWaveRingName);
        setenv("EW_PARAMS", mEarthwormParametersDirectory.c_str(), true);
        setenv("EW_INSTALLATION", mEarthwormInstallation.c_str(), true);
        mEarthwormWaveRing
            = std::make_shared<UMPS::Earthworm::WaveRing> (mLogger);
        mEarthwormWaveRing->connect(mEarthwormWaveRingName, mEarthwormWait);
        if (flushRing){mEarthwormWaveRing->flush();}
        mLogger->debug("Attach to earthworm ring: " + mEarthwormWaveRingName);
    }
    /// @brief Gets the command line input options as a string.
    std::string getInputOptions() const
    {
        std::string commands;
        commands = "Commands:\n";
        commands = commands + "   quit   Exits the program.\n";
        commands = commands + "   help   Displays this message.\n";
        return commands;
    }
    std::string getInputLine() const
    {
        return getName() + "$";
    }
    void processTerminal(const bool runInteractive = true)
    {
        while (IModule::keepRunning())
        {
            if (runInteractive)
            {
                std::string command;
                std::cout << getInputLine();
                std::cin >> command;
                if (command == "quit")
                {
                    stop();
                }
                else
                {
                    std::cout << std::endl;
                    if (command != "help")
                    {
                        std::cout << "Unknown command: " << command << std::endl;
                        std::cout << std::endl;
                    }
                    std::cout << getInputOptions();
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        mLogger->debug("Terminal management thread exiting...");
    }
    /// @brief Broadcasts the wave packets
    void start() override
    {
        stop(); // Make sure threads are stopped
        // Create the additional processes
        //std::unique_ptr<UMPS::Modules::IProcess> earthwormBroadcast
        //    = std::make_unique<BroadcastPackets> (mPacketPublisher,
        //                                          mEarthwormWaveRing, mLogger);
        //addProcess(std::move(earthwormBroadcast));
        IModule::start();
        mEarthwormBroadcast
            = std::make_shared<BroadcastPackets> (mPacketPublisher,
                                                  mEarthwormWaveRing, mLogger);
        mEarthwormBroadcast->start();
        //mTerminalThread = std::thread(&Module::processTerminal, this);
    }
    void stop() override
    {
        IModule::stop();
        //if (mBroadcastThread.joinable()){mBroadcastThread.join();}
        //if (mTerminalThread.joinable()){mTerminalThread.join();}
        if (mEarthwormBroadcast != nullptr){mEarthwormBroadcast->stop();}
        mEarthwormBroadcast = nullptr;
    }
    /// @brief Runs the main loop
//private:
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::shared_ptr<UMPS::Modules::ProcessManager> mProcessManager{nullptr};
    std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Publisher>
         mPacketPublisher{nullptr};
    std::shared_ptr<UMPS::Earthworm::WaveRing> mEarthwormWaveRing{nullptr};
    std::shared_ptr<BroadcastPackets> mEarthwormBroadcast{nullptr};
    std::string mEarthwormParametersDirectory{
         "/opt/earthworm/run_working/params/"};
    std::string mEarthwormInstallation{"INST_UNKNOWN"};
    std::string mEarthwormWaveRingName{"WAVE_RING"};
    std::string mDataPacketBroadcastName{"DataPacket"};
    int mEarthwormWait{0};
};
*/

std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[]);
//ProgramOptions parseInitializationFile(const std::string &iniFile);

int main(int argc, char *argv[])
{
    // Get the ini file from the command line
    bool runInteractive{false};
    std::string iniFile;
    try 
    {
        auto commandLineOptions = parseCommandLineOptions(argc, argv);
        iniFile = commandLineOptions.first;
        runInteractive = commandLineOptions.second;
        if (iniFile.empty()){return EXIT_SUCCESS;}
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Parse the initialization file
    ProgramOptions programOptions;
    try
    {
        programOptions.parseInitializationFile(iniFile, "General");
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Create the logger
    constexpr int hour = 0;
    constexpr int minute = 0;
    auto logger = createLogger(programOptions.mModuleName,
                               programOptions.mLogFileDirectory,
                               programOptions.mVerbosity,
                               hour, minute);
    // Initialize the wavering

    // This module only needs one context.  There's not much data to move.
    auto context = std::make_shared<UMPS::Messaging::Context> (1);
    // Initialize the modules
    logger->info("Initializing processes...");
    UMPS::Modules::ProcessManager processManager(logger);
    try
    {
        logger->debug("Connecting to uOperator...");
        const std::string operatorSection{"uOperator"};
        auto uOperator = UCI::createRequestor(iniFile, operatorSection,
                                              context, logger);
        programOptions.mZAPOptions = uOperator->getZAPOptions();

        logger->debug("Creating heartbeat process...");
        auto heartbeat = UHeartbeat::createHeartbeatProcess(*uOperator, iniFile,
                                                            "Heartbeat",
                                                            context, logger);
        processManager.insert(std::move(heartbeat));

        logger->debug("Creating packet broadcast process...");
        auto packetAddress
            = uOperator->getProxyBroadcastFrontendDetails(
                 programOptions.mDataPacketBroadcastName).getAddress();
        UMPS::ProxyBroadcasts::DataPacket::PublisherOptions
            packetPublisherOptions;
        auto packetPublisher = std::make_unique<UMPS::ProxyBroadcasts::DataPacket::Publisher> (context, logger);
        packetPublisherOptions.setAddress(packetAddress);
        packetPublisherOptions.setZAPOptions(programOptions.mZAPOptions);
        packetPublisher->initialize(packetPublisherOptions);

        constexpr bool flushRing{true};
        auto earthwormReader = createEarthwormWaveRingReader(programOptions,
                                                             logger,
                                                             flushRing);

        auto broadcastProcess 
            = std::make_unique<BroadcastPackets> (programOptions.mModuleName,
                                                  std::move(packetPublisher), 
                                                  std::move(earthwormReader),
                                                  logger);
        processManager.insert(std::move(broadcastProcess)); 
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        logger->error(e.what());
        return EXIT_FAILURE;
    }
    // Start the modules
    logger->info("Starting processes...");
    try
    {
        processManager.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        logger->error(e.what());
        return EXIT_FAILURE;
    }
    // Start the command queue

    // Run interactive
    while (processManager.isRunning())
    {
        if (runInteractive)
        {
            std::string command;
            std::cout << getInputLine();
            std::cin >> command;
            if (command == "quit")
            {
                processManager.stop();
            }
            else
            {
                std::cout << std::endl;
                if (command != "help")
                {
                    std::cout << "Unknown command: " << command << std::endl;
                    std::cout << std::endl;
                }
                std::cout << getInputOptions();
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    logger->info("Exiting...");
    //processManager.stop();
    return EXIT_SUCCESS;
/*
    // Initialize processes
     
    // Attach to module

    // Run module

    // Initialize the module
    Module module;
    try
    {
        module.parseInitializationFile(iniFile);
        module.createLogger();
        //module.createHeartbeatProcess();
    }
    catch (const std::exception &e)
    { 
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Connect to earthworm ring and uOperator
    try
    {
        module.connect();
        constexpr bool flushRing = true;
        module.attachToEarthwormWaveRing(flushRing);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
module.start();
module.processTerminal(runInteractive);
*/
//getchar();
//module.stop();
/*
try
{
module.start();
}
catch (const std::exception &e)
{
std::cerr << e.what() << std::endl;
}
*/
return 0;
/*
    try
    {
        module.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
*/
/*
    // Parse the ini file options
    std::cout << "Reading initialization file: " << iniFile << std::endl;
    ProgramOptions options;
    try 
    {
        options = parseInitializationFile(iniFile);
    }
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Create the application's logger
    constexpr int hour = 0;
    constexpr int minute = 0;
    auto waveRingLogFileName = options.logFileDirectory / "waveRing.log";
    UMPS::Logging::SpdLog logger;
    logger.initialize("WaveRing",
                      waveRingLogFileName,
                      UMPS::Logging::Level::DEBUG,
                      hour, minute);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::SpdLog> (logger);
    // Get the connection details
    UCI::Requestor request;
    request.initialize(options.mConnectionInformationRequestOptions);
    auto connectionDetails = request.getAllConnectionDetails();
    // Throws since we need this address
    auto packetAddress = request.getProxyBroadcastFrontendDetails(
                               options.dataBroadcastName).getAddress();
    std::string heartbeatAddress;
    try
    {
        heartbeatAddress = request.getProxyBroadcastFrontendDetails(
                               options.heartbeatBroadcastName).getAddress();
    }
    catch (const std::exception &e)
    {
        logger.info(e.what());
    }  
    // Connect to proxy
    UMPS::ProxyBroadcasts::DataPacket::PublisherOptions publisherOptions;
    publisherOptions.setAddress(packetAddress);
    publisherOptions.setZAPOptions(options.mZAPOptions);
    auto publisher
        = std::make_shared<UMPS::ProxyBroadcasts::DataPacket::Publisher>
          (loggerPtr);
    publisher->initialize(publisherOptions);
#ifndef NDEBUG
    assert(publisher->isInitialized());
#endif
    // Attach to the wave ring
    logger.info("Attaching to earthworm ring: "
              + options.earthwormWaveRingName);
    setenv("EW_PARAMS", options.earthwormParametersDirectory.c_str(), true);
    setenv("EW_INSTALLATION", options.earthwormInstallation.c_str(), true);
    auto waveRing
        = std::make_shared<UMPS::Earthworm::WaveRing> (loggerPtr);
    try
    {
        waveRing->connect(options.earthwormWaveRingName, options.earthwormWait);
    }
    catch (const std::exception &e)
    {
        logger.error("Error attaching to wave ring: " + std::string(e.what()));
        return EXIT_FAILURE;
    }
    waveRing->flush();
*/

/*
    BroadcastPackets broadcastPackets(module.mPacketPublisher, module.mEarthwormWaveRing, module.mLogger);
    std::thread broadcastThread(&BroadcastPackets::run, &broadcastPackets);

    if (runInteractive)
    {
        while (true)
        {
            std::string command;
            std::cout << "broadcastWaveRing$";
            std::cin >> command;
            if (command == "quit")
            {
                broadcastPackets.stop();
                break;
            }
            else
            {
                std::cout << std::endl;
                if (command != "help")
                {
                    std::cout << "Unknown command: " << command << std::endl;
                    std::cout << std::endl;
                }
                std::cout << "Commands: " << std::endl;
                std::cout << "  quit  Exits the program" << std::endl;
                std::cout << "  help  Prints this message" << std::endl;
            }
        }
        module.mLogger->info("Exiting...");
    }
    broadcastThread.join(); 
    module.mLogger->info("Program finished");
*/
    return EXIT_SUCCESS;
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
/// Read the program options from the command line
std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[])
{
    std::string iniFile;
    bool runInteractive = true;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Produce help message")
        ("background", "Setting this is useful for running the program as a background process - e.g., with nohup")
        ("ini",  boost::program_options::value<std::string> (), 
                 "Defines the initialization file for this module");
    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << MODULE_NAME
                  << " is for broadcasting traceBuf2 packets "
                  << "on an Earthworm wave ring to UMPS."
                  << std::endl << std::endl;
        std::cout << desc << std::endl;
        return std::pair{iniFile, runInteractive};
    }
    if (vm.count("ini"))
    {
        iniFile = vm["ini"].as<std::string>();
        if (!std::filesystem::exists(iniFile))
        {
            throw std::runtime_error("Initialization file: " + iniFile
                                   + " does not exist");
        }
    }
    else
    {
        throw std::runtime_error("Initialization file was not set");
    }
    if (vm.count("background")){runInteractive = false;}
    return std::pair{iniFile, runInteractive};
}

/*
/// Parse the ini file
ProgramOptions parseInitializationFile(const std::string &iniFile)
{
    ProgramOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    //------------------------------ General ---------------------------------//
    options.mLogFileDirectory = propertyTree.get<std::string>
        ("WaveRing.logFileDirectory", options.mLogFileDirectory.string());
    if (!options.mLogFileDirectory.empty() &&
        !std::filesystem::exists(options.mLogFileDirectory))
    {
        std::cout << "Creating log file directory: "
                  << options.mLogFileDirectory << std::endl;
        if (!std::filesystem::create_directories(options.mLogFileDirectory))
        {
            throw std::runtime_error("Failed to make log directory");
        }
    }
    //------------------------------ Operator --------------------------------//
    UCI::RequestorOptions requestOptions;
    requestOptions.parseInitializationFile(iniFile);
    options.mConnectionInformationRequestOptions = requestOptions;
    options.operatorAddress = requestOptions.getRequestOptions().getAddress();
    options.mZAPOptions = requestOptions.getRequestOptions().getZAPOptions();
    //------------------------------ Earthworm -------------------------------//
    // EW_PARAMS environment variable
    options.earthwormParametersDirectory = propertyTree.get<std::string>
        ("Earthworm.ewParams", options.mEarthwormParametersDirectory);
    if (!std::filesystem::exists(options.mEarthwormParametersDirectory))
    {   
        throw std::runtime_error("Earthworm parameters directory: " 
                               + options.mEarthwormParametersDirectory
                               + " does not exist");
    }   
    // EW_INST environment variable
    options.mEarthwormInstallation = propertyTree.get<std::string>
        ("Earthworm.ewInstallation", options.mEarthwormInstallation);
    // Earthworm wave ring
    options.mEarthwormWaveRingName = propertyTree.get<std::string>
        ("Earthworm.waveRingName", options.mEarthwormWaveRingName);
    // Wait after reading
    options.mEarthwormWait = propertyTree.get<int> ("Earthworm.wait",
                                                    options.mEarthwormWait);
    if (options.mEarthwormWait < 0)
    {   
        std::cerr << "Setting wait time to 0" << std::endl;
        options.mEarthwormWait = 0;
    }   
    //------------------------------------------------------------------------//
    return options;
}
*/
