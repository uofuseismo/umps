#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <map>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <filesystem>
#include "umps/services/incrementer/service.hpp"
#include "umps/services/incrementer/parameters.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/spdlog.hpp"

struct ProgramOptions
{
    std::vector<UMPS::Services::Incrementer::Parameters> mIncrementerParameters;
    std::string mLogDirectory = "./logs";
};

struct Modules
{
    std::vector<UMPS::Services::Incrementer::Service> mIncrementers;
};

ProgramOptions parseIniFile(const std::string &iniFile);
std::string parseCommandLineOptions(int argc, char *argv[]);

///-------------------------------------------------------------------------///
///                                 Main Program                            ///
///-------------------------------------------------------------------------///
int main(int argc, char *argv[])
{
    // Get the ini file from the command line
    std::string iniFile;
    try
    {
        iniFile = parseCommandLineOptions(argc, argv);
        if (iniFile.empty()){return EXIT_SUCCESS;}
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Parse the ini file options 
    std::cout << "Reading initialization file: " << iniFile << std::endl;
    ProgramOptions options;
    try
    {
        options = parseIniFile(iniFile);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Initialize the services
    Modules modules;
    modules.mIncrementers.reserve(options.mIncrementerParameters.size());
    for (const auto &parameters : options.mIncrementerParameters)
    {
        auto modulesName = "incrementer_" + parameters.getName();
        auto logFileName = options.mLogDirectory + "/" + modulesName + ".log";
        const int hour = 0;
        const int minute = 0;
        UMPS::Logging::SpdLog logger;
        logger.initialize(modulesName, logFileName,
                          parameters.getVerbosity(), hour, minute);
        std::shared_ptr<UMPS::Logging::ILog> loggerPtr
           = std::make_shared<UMPS::Logging::SpdLog> (logger);
        UMPS::Services::Incrementer::Service service(loggerPtr);
        try
        {
            service.initialize(parameters);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to initialize incrementer service"
                      << std::endl;
        }
        modules.mIncrementers.push_back(std::move(service));
    }
    // Now start the services - a thread per service
    std::cout << "Starting incrementer services..." << std::endl;
    std::vector<std::thread> threads;
    for (auto &module : modules.mIncrementers)
    {
        try
        {
            std::thread t(&UMPS::Services::Incrementer::Service::start,
                          &module);
            threads.push_back(std::move(t));
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            continue;
        }
    }

    // Main program loop
    while (true)
    {
        std::cout << std::endl << "uOperator$";
        std::string command;
        std::cin >> command;
        if (command == "help")
        {
            std::cout << "Commands:" << std::endl;
            std::cout << "  quit - exits the program" << std::endl;
            std::cout << "  help - shows this message" << std::endl;
            std::cout << "  list - lists the running services" << std::endl;
        }
        else if (command == "quit")
        {
            std::cout << "Quit command received" << std::endl;
            break;
        } 
        else if (command == "list")
        {
            std::cout << "Incrementers:" << std::endl;
            for (const auto &module : modules.mIncrementers)
            {
                try
                {
                    std::cout << "Service: " << module.getName()
                              << " available at: "
                              << module.getRequestAddress() << std::endl;
                }
                catch (const std::exception &e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Unknown command: " << command;
        }
//        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shut down the services
    std::cout << "Stopping incrementer services..." << std::endl;
    for (auto &module : modules.mIncrementers)
    {
        module.stop();
    }
    // Join the threads
    for (auto &thread : threads)
    {
        if (thread.joinable()){thread.join();}
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

///--------------------------------------------------------------------------///
/// Parse the ini file
ProgramOptions parseIniFile(const std::string &iniFile)
{
    ProgramOptions options;
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Parse the general properties
    options.mLogDirectory = propertyTree.get<std::string>
        ("uOperator.logFileDirectory", options.mLogDirectory);
    if (!std::filesystem::exists(options.mLogDirectory))
    {
        std::cout << "Creating log file directory: "
                  << options.mLogDirectory << std::endl;
        if (!std::filesystem::create_directories(options.mLogDirectory))
        {
            throw std::runtime_error("Failed to make log directory");
        }
    }

    // First get all the counters
    std::vector<std::string> counters;
    for (const auto &p : propertyTree)
    {
        if (p.first.find("Counters") != std::string::npos)
        {
            auto lExists = false;
            for (const auto &c : counters)
            {
                if (c == p.first)
                {
                    std::cerr << "Warning!  Counter: " << c
                              << " already exists; skipping..."
                              << std::endl;
                    lExists = true;
                    break;
                }
            }
            if (!lExists){counters.push_back(p.first);}
        }
    }
    // Parse the options for each counter
    for (const auto &counter : counters)
    {
        UMPS::Services::Incrementer::Parameters counterOptions;
        try
        {
            counterOptions.parseInitializationFile(iniFile, counter);
            options.mIncrementerParameters.push_back(std::move(counterOptions));
        }
        catch (const std::exception &e)
        {
           std::cerr << "Failed to read incrementer options for: " << counter
                     << " Failed with:\n" << e.what() << std::endl;
        }
    }        
//std::cout << p.first << std::endl;
/*
    // EW_PARAMS environment variable
    options.earthwormParametersDirectory = propertyTree.get<std::string>
        ("Earthworm.ewParams", options.earthwormParametersDirectory);
    if (!std::filesystem::exists(options.earthwormParametersDirectory))
    {   
        throw std::runtime_error("Earthworm parameters directory: " 
                               + options.earthwormParametersDirectory
                               + " does not exist");
    }   
    // Create the counters... 
*/
    return options;
}
