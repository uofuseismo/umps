#include <iostream>
#include <string>
#include <cmath>
#include <thread>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/authentication/zapOptions.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/requestor.hpp"
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/socketDetails/proxy.hpp"
#include "umps/services/connectionInformation/socketDetails/dealer.hpp"
#include "umps/services/connectionInformation/socketDetails/xPublisher.hpp"
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/proxyServices/incrementer/service.hpp"

namespace UIncrementer = UMPS::ProxyServices::Incrementer;
namespace UAuth = UMPS::Authentication;
namespace UCI = UMPS::Services::ConnectionInformation;


struct ProgramOptions
{
    UIncrementer::Options mIncrementerOptions;
    UCI::RequestorOptions mConnectionInformationRequestOptions;
    //UPacketCache::ReplierOptions mPacketCacheReplyOptions;
    UAuth::ZAPOptions mZAPOptions;
    std::string mLogFileDirectory = "logs";
    std::string operatorAddress;
    std::string proxyServiceName = "Incrementer";
    UMPS::Logging::Level mVerbosity = UMPS::Logging::Level::INFO;
};


std::string parseOptions(int argc, char *argv[]);
ProgramOptions parseInitializationFile(const std::string &iniFile);

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
        options = parseInitializationFile(iniFile);
    }
    catch (const std::exception &e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    auto zapOptions = options.mZAPOptions;
    // Create logger
    UMPS::Logging::StdOut logger;
    logger.setLevel(options.mVerbosity); //UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
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
        std::cout << "incrementer$";
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
            std::cout << "Commands: " << std::endl;
            std::cout << "  quit      Exits the program" << std::endl;
            std::cout << "  help      Prints this message" << std::endl;
        }
    }
    logger.info("Stopping services...");
    service.stop();
    logger.info("Program finished");
    return EXIT_SUCCESS;
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
