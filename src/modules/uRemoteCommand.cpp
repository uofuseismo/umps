#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#ifndef NDEBUG
#include <cassert>
#endif
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/standardOut.hpp"
#include "umps/messaging/context.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/socketDetails/router.hpp"
#include "umps/proxyServices/command/requestorOptions.hpp"
#include "umps/proxyServices/command/requestor.hpp"
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/terminateResponse.hpp"
/*
#include "umps/services/command/moduleDetails.hpp"
*/
/*
#include "umps/services/command/requestorOptions.hpp"
#include "umps/services/command/terminateResponse.hpp"
*/
#define DASHES "-----------------------------------------------------"
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

[[nodiscard]] std::string parseCommandLineOptions(int argc, char *argv[]);

/// @result The logger for this application.
std::shared_ptr<UMPS::Logging::ILog>
    createLogger(const UMPS::Logging::Level verbosity
                    = UMPS::Logging::Level::Info)
{
    auto logger = std::make_shared<UMPS::Logging::StandardOut> (verbosity);
    return logger;
}

/// @brief Defines the module options.
struct ProgramOptions
{
    /// @brief Load the module options from an initialization file.
    void parseInitializationFile(const std::string &iniFile)
    {
        boost::property_tree::ptree propertyTree;
        boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
        //----------------------------- General ------------------------------//
        /*
        // Module name
        mModuleName
            = propertyTree.get<std::string> ("General.moduleName",
                                             mModuleName);
        if (mModuleName.empty())
        {
            throw std::runtime_error("Module name not defined");
        }
        */
        // The name of the service module registry service
        mServiceName = propertyTree.get<std::string>
                       ("General.moduleRegistryServiceName", mServiceName);
        // The address of the service
        mFrontendAddress
            = propertyTree.get<std::string>
              ("General.moduleRegistryFrontendAddress", mFrontendAddress);
        if (mServiceName.empty() && mFrontendAddress.empty())
        {
            throw std::invalid_argument("Must specify service name or address");
        }
        // Verbosity
        mVerbosity = static_cast<UMPS::Logging::Level>
                     (propertyTree.get<int> ("General.verbose",
                                             static_cast<int> (mVerbosity)));
        // Log file directory
        /*
        mLogFileDirectory
            = propertyTree.get<std::string> ("General.logFileDirectory",
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
        */
    }
///private:
    //std::filesystem::path mLogFileDirectory{"/var/log/urts"};
    UAuth::ZAPOptions mZAPOptions;
    UMPS::Logging::Level mVerbosity{UMPS::Logging::Level::Info};
    std::string mFrontendAddress;
    std::string mServiceName{"ModuleRegistry"};
};


int main(int argc, char *argv[])
{
    // Get the ini file from the command line
    std::string iniFile;
    try 
    {
        iniFile = ::parseCommandLineOptions(argc, argv);
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
        programOptions.parseInitializationFile(iniFile);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Create the logger
    auto logger = createLogger();
    // This module only needs one context.  There's not much data to move.
    auto context = std::make_shared<UMPS::Messaging::Context> (1);
    // Connect to the operator and get the address of the frontend
    if (programOptions.mFrontendAddress.empty())
    {
        logger->debug("Connecting to uOperator...");
        try
        {
            const std::string operatorSection{"uOperator"};
            auto uOperator = UCI::createRequestor(iniFile, operatorSection,
                                                  context, logger);
            programOptions.mZAPOptions = uOperator->getZAPOptions();
            // Get the name of the frontend address
            programOptions.mFrontendAddress
                = uOperator->getProxyServiceFrontendDetails(
                     programOptions.mServiceName).getAddress();
        }
        catch (const std::exception &e)
        {
            logger->error(e.what());
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    // query connections
std::cout << programOptions.mFrontendAddress << std::endl;
    UMPS::ProxyServices::Command::Requestor requestor(context, logger);
    try
    {
        UMPS::ProxyServices::Command::RequestorOptions requestOptions;
        requestOptions.setAddress(programOptions.mFrontendAddress);
        requestOptions.setZAPOptions(programOptions.mZAPOptions);
        requestor.initialize(requestOptions);
    }
    catch (const std::exception &e)
    {
        logger->error(e.what());
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
#ifndef NDEBUG
    assert(requestor.isInitialized());
#endif
    while (true)
    {   
        std::string command;
        std::cout << "uRemoteCommand$ ";
        std::cin >> std::ws;
        std::getline(std::cin, command);
        if (command == "quit")
        {
            break;
        }
        else if (command == "list")
        {
            auto modules = requestor.getAvailableModules();
            if (modules)
            {
                for (const auto &module : *modules)
                {
                    std::cout << module <<std::endl;
                }
            }
        }
        else if (command.find("connect") == 0)
        {
            auto modules = requestor.getAvailableModules();
            // Get the module name
            std::vector<std::string> splitCommand;
            boost::split(splitCommand, command, ::isspace);
            uint16_t instance{0};
            if (splitCommand.size() < 2 || splitCommand.size() > 4)
            {
                std::cerr << "Appropriate usage is: "
                          << "   connect [ModuleName]" << std::endl
                          << "   connect [ModuleName] [Instance]" << std::endl;
                continue;
            }
            auto moduleName = splitCommand.at(1);
            if (splitCommand.size() == 3)
            {
                try
                {
                    instance
                        = static_cast<uint16_t> (std::stoi(splitCommand.at(2)));
                }
                catch (const std::exception &e)
                {
                    instance = 0;
                    std::cerr << e.what() << std::endl;
                    continue;
                }
            }
            // Is the module name in the list?
            bool lMatch{false};
            for (const auto &module : *modules)
            {
                if (moduleName == module.getName()){lMatch = true;}
            }
            if (!lMatch)
            {
                std::cerr << "Module: " << moduleName;
                if (splitCommand.size() == 3)
                {
                    std::cerr << " (Instance " << instance << ")";
                }
                std::cerr << " does not exist." << std::endl;
                continue;
            }
            try
            {
               auto commands = requestor.getCommands(moduleName, instance);
               std::cout << commands->getCommands() << std::endl;
               std::cout << DASHES << std::endl;
            }
            catch (const std::exception &e)
            {
               logger->error(e.what());
            }
            std::cout << DASHES << std::endl;
            std::cout << "To terminate session use: hangup" << std::endl;
            bool maintainConnection{true};
            while (maintainConnection)
            {
                std::cout << moduleName << "$ ";
                std::cin >> std::ws;
                std::getline(std::cin, command);
                if (command == "hangup")
                {
                    std::cout << "Hanging up on " << moduleName << std::endl;
                    maintainConnection = false;
                }
                else if (command == "quit")
                {
                    namespace USCommand = UMPS::Services::Command;
                    maintainConnection = false;
                    auto response = requestor.issueTerminateCommand(moduleName,
                                                                    instance);
                    if (response->getReturnCode() !=
                        USCommand::TerminateResponse::ReturnCode::Success)
                    {
                        std::cerr << "Error terminating: " << moduleName
                                  << std::endl;
                    }
                }
                else
                {
                    UMPS::Services::Command::CommandRequest commandRequest;
                    commandRequest.setCommand(command);
                    try
                    {
                        auto response = requestor.issueCommand(moduleName,
                                                               instance,
                                                               commandRequest);
                        std::cout << response->getResponse() << std::endl;
                        if (command == "quit"){maintainConnection = false;}
                    }
                    catch (const std::exception &e)
                    {
                        logger->error(e.what());
                        maintainConnection = false;
                    }
                }
            }

            std::cout << DASHES << std::endl;
        }
        else
        {
            if (command != "help")
            {
                std::cout << "Unhandled command: " << command << std::endl;
            }
            std::cout << "Options:" << std::endl;
            std::cout << "   help                  Prints this message."
                      << std::endl;
            std::cout << "   list                  Lists the running modules."
                      << std::endl;
            std::cout << "   connect [ModuleName]  Connects to the module."
                      << std::endl;
            std::cout << "   quit                  Exits this application."
                      << std::endl;
        }
        std::cout << std::endl;
    }
    return EXIT_SUCCESS;
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
/// Read the program options from the command line
std::string parseCommandLineOptions(int argc, char *argv[])
{
    std::string iniFile;
    boost::program_options::options_description desc(
R"""(
uRemoteCommand allows a super user to remotely interact with other running
modules.  Example usage:

    uRemoteCommand --ini=remoteCommand.ini

Allowed options
)""");
    desc.add_options()
        ("help", "Produces this help message")
        ("ini",  boost::program_options::value<std::string> (),
                 "The initialization file for this executable");
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
