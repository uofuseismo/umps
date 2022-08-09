#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#ifndef NDEBUG
#include <cassert>
#endif
#include <zmq.hpp>
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
//#include "umps/proxyBroadcasts/heartbeat/publisher.hpp"
//#include "umps/proxyBroadcasts/heartbeat/publisherOptions.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
//#include "umps/services/connectionInformation/getConnections.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/connectionInformation/socketDetails/xSubscriber.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
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

struct ProgramOptions
{
    UCI::RequestorOptions mConnectionInformationRequestOptions;
    UAuth::ZAPOptions mZAPOptions;
    std::string earthwormParametersDirectory
        = "/opt/earthworm/run_working/params/";
    std::string earthwormInstallation = "INST_UNKNOWN"; 
    std::string earthwormWaveRingName = "WAVE_RING";
    std::string operatorAddress;
    std::string dataBroadcastName = "DataPacket";
    std::string heartbeatBroadcastName = "Heartbeat";
    std::filesystem::path logFileDirectory = "logs";
    int earthwormWait = 0;
    std::chrono::seconds heartBeatInterval{30};
};

class BroadcastPackets : public UMPS::Modules::IProcess
{
public:
    BroadcastPackets() = default;
    BroadcastPackets(
        std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Publisher> &packetPublisher,
        std::shared_ptr<UMPS::Earthworm::WaveRing> &waveRing,
        std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mPacketPublisher(packetPublisher),
        mWaveRing(waveRing),
        mLogger(logger)
    {
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
    }
    /// @brief Starts the process.
    void start() override
    {
        stop();
        setRunning(true);
        mLogger->debug("Starting the waveRing broadcast thread...");
        mBroadcastThread = std::thread(&BroadcastPackets::run,
                                       this);
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
        constexpr std::chrono::seconds oneSecond{1}; 
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
                mLogger->error("Failed reading wave ring:\n"
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
            if (duration < oneSecond)
            {
                auto wait = oneSecond - duration;
                std::this_thread::sleep_for(wait);
            }
            //std::cout << "Sent: " << nSent << std::endl;
        }
        mLogger->debug("Earthworm broadcast thread is terminating");
    }
    mutable std::mutex mMutex;
    std::thread mBroadcastThread;
    std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Publisher> mPacketPublisher;
    std::shared_ptr<UMPS::Earthworm::WaveRing> mWaveRing;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    bool mKeepRunning = true;
};

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
/*
        // Create the additional processes
        std::unique_ptr<UMPS::Modules::IProcess> earthwormBroadcast
            = std::make_unique<BroadcastPackets> (mPacketPublisher,
                                                  mEarthwormWaveRing, mLogger);
        addProcess(std::move(earthwormBroadcast));
*/
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
    std::shared_ptr<UMPS::ProxyBroadcasts::DataPacket::Publisher>
         mPacketPublisher{nullptr};
    std::shared_ptr<UMPS::Earthworm::WaveRing> mEarthwormWaveRing{nullptr};
    std::shared_ptr<BroadcastPackets> mEarthwormBroadcast{nullptr};
    std::string mEarthwormParametersDirectory
        = "/opt/earthworm/run_working/params/";
    std::string mEarthwormInstallation = "INST_UNKNOWN";
    std::string mEarthwormWaveRingName = "WAVE_RING";
    std::string mDataPacketBroadcastName = "DataPacket";
    //std::thread mBroadcastThread;
    //std::thread mTerminalThread;
    int mEarthwormWait = 0;
};

std::pair<std::string, bool> parseCommandLineOptions(int argc, char *argv[]);
ProgramOptions parseInitializationFile(const std::string &iniFile);

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
        ("help", "produce help message")
        ("background", "Setting this is useful for running the program as a background process - e.g., with nohup")
        ("ini",  boost::program_options::value<std::string> (), 
                 "Defines the initialization file for this module");
    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
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

/// Parse the ini file
ProgramOptions parseInitializationFile(const std::string &iniFile)
{
    ProgramOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    //------------------------------ General ---------------------------------//
    options.logFileDirectory = propertyTree.get<std::string>
        ("WaveRing.logFileDirectory", options.logFileDirectory.string());
    if (!options.logFileDirectory.empty() &&
        !std::filesystem::exists(options.logFileDirectory))
    {
        std::cout << "Creating log file directory: "
                  << options.logFileDirectory << std::endl;
        if (!std::filesystem::create_directories(options.logFileDirectory))
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

/*
    options.operatorAddress = propertyTree.get<std::string>
        ("uOperator.ipAddress", options.operatorAddress);
    if (isEmpty(options.operatorAddress))
    {
        throw std::runtime_error("Operator address not set");
    } 
    options.mZAPOptions
        = UMPS::Modules::Operator::readZAPClientOptions(propertyTree);
*/
/*
    // Security level
    auto securityLevel = static_cast<UAuth::SecurityLevel>
       (propertyTree.get<int> ("uOperator.securityLevel",
                    static_cast<int> (options.mZAPOptions.getSecurityLevel())));
    if (static_cast<int> (securityLevel) < 0 ||
        static_cast<int> (securityLevel) > 4)
    {
        throw std::invalid_argument("Security level must be in range [0,4]");
    }
    // Define ZAP options
    options.mZAPOptions.setGrasslandsClient();
    if (securityLevel == UAuth::SecurityLevel::GRASSLANDS)
    {
        options.mZAPOptions.setGrasslandsClient();
    }
    else if (securityLevel == UAuth::SecurityLevel::STRAWHOUSE)
    {
        options.mZAPOptions.setStrawhouseClient();
    }
    else if (securityLevel == UAuth::SecurityLevel::WOODHOUSE)
    {
        UAuth::Certificate::UserNameAndPassword credentials;
        //options.mZAPOptions.setWoodhouseClient();
    }
    else if (securityLevel == UAuth::SecurityLevel::STONEHOUSE)
    {
        auto serverPublicKeyFile
            = propertyTree.get<std::string> ("uOperator.serverPublicKeyFile");
        auto clientPublicKeyFile
            = propertyTree.get<std::string> ("uOperator.clientPublicKeyFile");
        auto clientPrivateKeyFile
            = propertyTree.get<std::string> ("uOperator.clientPrivateKeyFile");
        UAuth::Certificate::Keys serverKeys, clientKeys;
        serverKeys.loadFromTextFile(serverPublicKeyFile);
        clientKeys.loadFromTextFile(clientPublicKeyFile);
        clientKeys.loadFromTextFile(clientPrivateKeyFile);
        options.mZAPOptions.setStonehouseClient(serverKeys, clientKeys);
    }
    else
    {
        throw std::runtime_error("Unhandled security level");
    }
*/
    //------------------------------ Earthworm -------------------------------//
    // EW_PARAMS environment variable
    options.earthwormParametersDirectory = propertyTree.get<std::string>
        ("Earthworm.ewParams", options.earthwormParametersDirectory);
    if (!std::filesystem::exists(options.earthwormParametersDirectory))
    {   
        throw std::runtime_error("Earthworm parameters directory: " 
                               + options.earthwormParametersDirectory
                               + " does not exist");
    }   
    // EW_INST environment variable
    options.earthwormInstallation = propertyTree.get<std::string>
        ("Earthworm.ewInstallation", options.earthwormInstallation);
    // Earthworm wave ring
    options.earthwormWaveRingName = propertyTree.get<std::string>
        ("Earthworm.waveRingName", options.earthwormWaveRingName);
    // Wait after reading
    options.earthwormWait = propertyTree.get<int> ("Earthworm.wait",
                                                   options.earthwormWait);
    if (options.earthwormWait < 0)
    {   
        std::cerr << "Setting wait time to 0" << std::endl;
        options.earthwormWait = 0;
    }   
    //------------------------------------------------------------------------//
    return options;
}
