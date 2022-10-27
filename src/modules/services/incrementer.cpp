#include <iostream>
#include <string>
#include <cmath>
#include <mutex>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/dailyFile.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/modules/process.hpp"
#include "umps/modules/processManager.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcess.hpp"
#include "umps/proxyBroadcasts/heartbeat/publisherProcessOptions.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/service.hpp"
#include "umps/services/command/serviceOptions.hpp"
#include "umps/services/command/terminateRequest.hpp"
#include "umps/services/command/terminateResponse.hpp"
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/proxyServices/incrementer/service.hpp"

#define MODULE_NAME "Incrementer"

std::string parseOptions(int argc, char *argv[]);

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

/// @result The logger for this application.
[[nodiscard]] std::shared_ptr<UMPS::Logging::ILog>
    createLogger(const std::string &moduleName,
                 const std::filesystem::path logFileDirectory = "/var/log/umps",
                 const UMPS::Logging::Level verbosity = UMPS::Logging::Level::Info,
                 const int hour = 0, const int minute = 0)
{
    auto logFileName = moduleName + ".log";  
    auto fullLogFileName = logFileDirectory / logFileName;
    auto logger = std::make_shared<UMPS::Logging::DailyFile> (); 
    logger->initialize(moduleName,
                       fullLogFileName,
                       verbosity,
                       hour, minute);
    logger->info("Starting logging for " + moduleName);
    return logger;
}

namespace UIncrementer = UMPS::ProxyServices::Incrementer;
namespace UAuth = UMPS::Authentication;
namespace UCI = UMPS::Services::ConnectionInformation;

///--------------------------------------------------------------------------///
///                      Handle Different App Processes                      ///
///--------------------------------------------------------------------------///
class IncrementerProcess : public UMPS::Modules::IProcess
{
public:
    IncrementerProcess() = delete;
    IncrementerProcess(const std::string &moduleName,
                       std::unique_ptr<UIncrementer::Service> &&incrementer,
                       const std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mIncrementer(std::move(incrementer)),
        mLogger(logger)
    {
        namespace UCommand = UMPS::Services::Command;
        if (!mIncrementer->isInitialized())
        {
            throw std::invalid_argument("Incrementer service not initialized");
        }
        mLocalCommand = std::make_unique<UCommand::Service> (mLogger);
        UCommand::ServiceOptions localServiceOptions;
        localServiceOptions.setModuleName(moduleName);
        localServiceOptions.setCallback(
            std::bind(&IncrementerProcess::commandCallback,
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
    ~IncrementerProcess() override
    {   
        stop();
    }
    /// @result True indicates this should keep running
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
    [[nodiscard]] std::string getName() const noexcept override
    {
        return "Incrementer";
    }
    /// @brief Stops the process.
    void stop() override
    {
        setRunning(false);
        if (mIncrementer != nullptr)
        {
            if (mIncrementer->isRunning()){mIncrementer->stop();}
        }
        if (mLocalCommand != nullptr)
        {
            if (mLocalCommand->isRunning()){mLocalCommand->stop();}
        }
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
        mLogger->debug("Starting the incrementer service thread...");
        mIncrementer->start();
        mLogger->debug("Starting the local command proxy...");
        mLocalCommand->start();
    }
    /// Running?
    [[nodiscard]] bool isRunning() const noexcept override
    {
        return keepRunning();
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
        USC::TerminateRequest terminateRequest;
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
        else if (messageType == terminateRequest.getMessageType())
        {
            USC::TerminateResponse response;
            try
            {
                terminateRequest.fromMessage(
                    static_cast<const char *> (data), length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Failed to unpack terminate request");
            }
            issueStopCommand();
            response.setReturnCode(USC::TerminateResponse::ReturnCode::Success);
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
                issueStopCommand();
                response.setResponse("Bye!  But next time use the terminate command.");
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
///private:
    mutable std::mutex mMutex;
    std::thread mIncrementerThread;
    std::unique_ptr<UIncrementer::Service> mIncrementer{nullptr};
    std::unique_ptr<UMPS::Services::Command::Service> mLocalCommand{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    bool mKeepRunning{true};
    bool mInitialized{false};
};

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
    }
    UIncrementer::Options mIncrementerOptions;
    UCI::RequestorOptions mConnectionInformationRequestOptions;
    UAuth::ZAPOptions mZAPOptions;
    std::filesystem::path mLogFileDirectory{"/var/log/umps"};
    std::string operatorAddress;
    std::string proxyServiceName{"Incrementer"};
    std::string mModuleName{MODULE_NAME};
    UMPS::Logging::Level mVerbosity{UMPS::Logging::Level::INFO};
};


//ProgramOptions parseInitializationFile(const std::string &iniFile);

int main(int argc, char *argv[])
{
    // Get the program options
    std::string iniFile;
    try 
    {
        iniFile = parseOptions(argc, argv);
        if (iniFile.empty()){return EXIT_SUCCESS;}
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Read the configuration file
    ProgramOptions options;
    try 
    {
        //options = parseInitializationFile(iniFile);
        options.parseInitializationFile(iniFile);
    }
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    //auto zapOptions = options.mZAPOptions;
    // Create the logger
    constexpr int hour = 0;
    constexpr int minute = 0;
    auto logger = createLogger(options.mModuleName,
                               options.mLogFileDirectory,
                               options.mVerbosity,
                               hour, minute);
    // This module only needs one context.  There's not much to do.
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
        options.mZAPOptions = uOperator->getZAPOptions();

        namespace UHeartbeat = UMPS::ProxyBroadcasts::Heartbeat;
        logger->debug("Creating heartbeat process...");
        auto heartbeat = UHeartbeat::createHeartbeatProcess(*uOperator,
                                                            iniFile,
                                                            "Heartbeat",
                                                            context,
                                                            logger);
        processManager.insert(std::move(heartbeat));

        logger->debug("Creating incrementer process...");
        auto backendAddress = uOperator->getProxyServiceBackendDetails(
               options.proxyServiceName).getAddress();
        options.mIncrementerOptions.setBackendAddress(backendAddress); 
        options.mIncrementerOptions.setZAPOptions(options.mZAPOptions);
        auto incrementerService
            = std::make_unique<UIncrementer::Service> (context, logger);
        incrementerService->initialize(options.mIncrementerOptions);

        auto incrementer
            = std::make_unique<IncrementerProcess>
                (options.mModuleName,
                 std::move(incrementerService),
                 logger);
        processManager.insert(std::move(incrementer));
    }
    catch (const std::exception  &e)
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
    // Do something with the main thread
    logger->info("Starting main thread...");
    processManager.handleMainThread();

    return EXIT_SUCCESS;
/*
    // Get the connection details
    logger.info("Getting available services...");
    std::string incrementerProxyBackendAddress;
    try
    {
        UCI::Requestor connectionInformation;
        connectionInformation.initialize(
            options.mConnectionInformationRequestOptions);
        incrementerProxyBackendAddress
            = connectionInformation.getProxyServiceBackendDetails(
                 options.proxyServiceName).getAddress();
        options.mIncrementerOptions.setBackendAddress(
            incrementerProxyBackendAddress);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        logger.error(std::string {e.what()});
        return EXIT_FAILURE;
    }
    // Set the connection details
    UIncrementer::Service service(loggerPtr);
    auto incrementerOptions = options.mIncrementerOptions; 
    try
    {
        logger.info("Initializing the incrementer...");
        service.initialize(incrementerOptions);
#ifndef NDEBUG
        assert(service.isInitialized());
#endif
        logger.info("Starting the incrementer service...");
        service.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        logger.error(std::string {e.what()});
        return EXIT_FAILURE;
    } 
    // Monitor commands from stdin
    while (true)
    {
        std::string command;
        std::cout << getInputLine();
        std::cin >> command;
        if (command == "quit")
        {
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
            std::cout << getInputOptions() << std::endl;
        }
    }
    logger.info("Stopping services...");
    service.stop();
    logger.info("Program finished");
    return EXIT_SUCCESS;
*/
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
/// Read the program options from the command line
std::string parseOptions(int argc, char *argv[])
{
    std::string iniFile;
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("ini",  boost::program_options::value<std::string> (),
                 "Defines the initialization file for this module");
    boost::program_options::variables_map vm; 
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return iniFile;
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
    return iniFile;
}

/*
ProgramOptions parseInitializationFile(const std::string &iniFile)
{
    ProgramOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    //--------------------------Incrementer Options--- -----------------------//
    const std::string section = "Incrementer";
    options.proxyServiceName = propertyTree.get<std::string>
                               (section + ".proxyServiceName", "");
    if (options.proxyServiceName.empty())
    {
        throw std::invalid_argument("proxyServiceName not set");
    }

    auto verbosity = propertyTree.get<int>
                     (section + ".verbose",
                      static_cast<int> (options.mVerbosity));
    verbosity = std::min(std::max(0, verbosity), 4);
    options.mVerbosity = static_cast<UMPS::Logging::Level> (verbosity);

    auto logFileDirectory = propertyTree.get<std::string>
                            (section + ".logFileDirectory",
                            options.mLogFileDirectory);
    if (logFileDirectory.empty())
    {
        logFileDirectory = std::string{std::filesystem::current_path()};
    }
    else
    {
        if (!std::filesystem::exists(logFileDirectory))
        {
            if (!std::filesystem::create_directories(logFileDirectory))
            {
                throw std::runtime_error("Failed to create log file directory: "
                                       + logFileDirectory);
            }
        }
    }
    options.mLogFileDirectory = logFileDirectory;
    //------------------------------ Operator --------------------------------//
    UCI::RequestorOptions requestOptions;
    requestOptions.parseInitializationFile(iniFile);
    options.operatorAddress = requestOptions.getRequestOptions().getAddress();
    options.mZAPOptions = requestOptions.getRequestOptions().getZAPOptions();
    options.mConnectionInformationRequestOptions = requestOptions;
    options.mConnectionInformationRequestOptions.setZAPOptions(
        options.mZAPOptions);
    options.mIncrementerOptions.setZAPOptions(
        options.mZAPOptions);

    return options;
}
*/
