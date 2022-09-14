#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/services/command/localModuleTable.hpp"
#include "umps/services/command/localModuleDetails.hpp"
#include "umps/services/command/localRequestor.hpp"
#include "umps/services/command/localRequestorOptions.hpp"


int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    std::shared_ptr<UMPS::Logging::ILog> logger
        = std::make_shared<UMPS::Logging::StdOut> ();
    auto context = std::make_shared<UMPS::Messaging::Context> (1);
    std::unique_ptr<UMPS::Services::Command::LocalRequestor> requestor{nullptr};
    UMPS::Services::Command::LocalModuleTable moduleTable;
    std::vector<UMPS::Services::Command::LocalModuleDetails> allModules;
    try
    {
        moduleTable.openReadOnly();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    while (true)
    {
        std::string command;
        std::cout << "uCommand$";
        std::cin >> std::ws;
        std::getline(std::cin, command);
        if (command == "quit")
        {
            break;
        }
        else if (command == "list")
        {
            allModules = moduleTable.queryAllModules();        
            for (const auto &m : allModules)
            {
                std::cout << m.getName() << std::endl;
            }
        }
        else if (command.find("connect") == 0)
        {
            allModules = moduleTable.queryAllModules();
            // Get the module name
            std::vector<std::string> splitCommand;
            boost::split(splitCommand, command, ::isspace);//any_of(std::is_space" "));
            if (splitCommand.size() != 2)
            {
                std::cerr << "Appropriate usage is: connect [ModuleName]" << std::endl;
                continue;
            }
            auto moduleName = splitCommand.at(1);
            // Check the module exists
            bool found = false;
            for (const auto &m : allModules)
            {
                if (moduleName == m.getName())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "Could not find module: " << moduleName << std::endl;
                continue;
            } 
            // Now connect
            UMPS::Services::Command::LocalRequestorOptions requestorOptions;
            requestorOptions.setModuleName(moduleName);
            requestor
                = std::make_unique<UMPS::Services::Command::LocalRequestor>
                  (context, logger);
            bool maintainConnection = true;
            try
            {
                requestor->initialize(requestorOptions);    
            }
            catch (const std::exception &e)
            {
                logger->error(std::string {e.what()});
                maintainConnection = false;
            }
            // Now begin interactive loop
            std::unique_ptr<UMPS::Services::Command::AvailableCommandsResponse>
                commands{nullptr};
            try
            {
                commands = requestor->getCommands();
                std::cout << commands->getCommands() << std::endl;
            }
            catch (const std::exception &e)
            {
                logger->error(e.what());
                maintainConnection = false;
            }
            // Connection established - interact with program
            std::cout << "-----------------------------------------------------"
                      << std::endl;
            std::cout << "To terminate session use: hangup" << std::endl;
            while (maintainConnection)
            {
                std::cout << moduleName << "$";
                std::cin >> std::ws;
                std::getline(std::cin, command);
                if (command == "hangup")
                {
                    maintainConnection = false;
                }
                else
                {
                    UMPS::Services::Command::CommandRequest commandRequest;
                    commandRequest.setCommand(command);
                    try
                    {
                        auto response = requestor->issueCommand(commandRequest); 
                        std::cout << response->getResponse() << std::endl;
                        if (command == "quit")
                        {
                            maintainConnection = false;
                        }
                    }
                    catch (const std::exception &e)
                    {
                        logger->error(e.what());
                        maintainConnection = false;
                    }
                }
            } 
            std::cout << "-----------------------------------------------------"
                      << std::endl;
        }
        else
        {
            if (command != "help")
            {
                std::cout << "Unhandled command: " << command << std::endl;
            }
            std::cout << "Options:" << std::endl;
            std::cout << "   help                  Prints this message." << std::endl;
            std::cout << "   list                  Lists the running modules." << std::endl;
            std::cout << "   connect [ModuleName]  Connects to the module." << std::endl;
            std::cout << "   quit                  Exits this application." << std::endl;
        }
        std::cout << std::endl;
    }
    return EXIT_SUCCESS;
}
