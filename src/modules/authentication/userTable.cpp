#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <nlohmann/json.hpp>
#include "umps/messaging/authentication/sqlite3Authenticator.hpp"
#include "umps/messaging/authentication/user.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "private/isEmpty.hpp"

namespace UAuth = UMPS::Messaging::Authentication;

struct ProgramOptions
{
    std::string mUserTable;
    int mError = 0;
};

ProgramOptions parseCommandLineOptions(int argc, char *argv[]);

[[nodiscard]] UAuth::User loadUserFromJSONFile(const std::string &fileName)
{
    UAuth::User user;
    std::ifstream inFile(fileName);
    nlohmann::json obj;
    inFile >> obj;
    std::string name = obj["Name"].get<std::string> (); 
    user.setName(name); // Throws
    std::string email = obj["Email"].get<std::string> (); 
    user.setEmail(email); // Throws
    std::string password = obj["Password"].get<std::string> (); 
    if (password.empty())
    {
    }
    //if (!password.empty()){user.setHashedPassword(password);}
    //std::string publicKey = obj["PublicKey"].get<std::string> (); 
    //if (static_cast<int> (publicKey.length()) == 40) 

    return user;
}

[[nodiscard]] UAuth::User getUserFromJSON()
{
    std::string fileName;
    std::cout << "Enter user JSON file" << std::endl << ">";
    std::cin >> fileName;
    if (!std::filesystem::exists(fileName))
    {
        throw std::invalid_argument("User JSON file does: "
                                  + fileName + " not exist");
    }
    return loadUserFromJSONFile(fileName);
}

std::string generalHelpMessage = R"""(
Available commands:

   (l)ist    Lists the users in the database.
   (a)dd     Adds a user.
   (u)pdate  Updates the users.
   (q)uit    Exits the program.
   (h)elp    Displays this message.
)""";

int main(int argc, char *argv[])
{
    auto options = parseCommandLineOptions(argc, argv);
    if (options.mError != 0)
    {
        if (options.mError == -1){return EXIT_SUCCESS;}
        std::cerr << "Execution failed" << std::endl; 
        return EXIT_FAILURE;
    }
    // Open the authenticator -> this can do table management
    UAuth::SQLite3Authenticator authenticator;
    bool createIfDoesNotExist = true;
    if (std::filesystem::exists(options.mUserTable))
    {
        std::cout << "Opening: " << options.mUserTable << std::endl;
        createIfDoesNotExist = false;
    }
    else
    {
        std::cout << "Creating: " << options.mUserTable << std::endl;
        createIfDoesNotExist = true;
    }
    // Open the users table
    try
    {
        authenticator.openUsersTable(options.mUserTable, createIfDoesNotExist);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } 


    std::cout << "Welcome to the User Table Manager!" << std::endl;
    std::cout << generalHelpMessage << std::endl;
    while (true)
    {
        std::cout << std::endl << ">";
        std::string command;
        std::cin >> command;

        if (command == "h" || command == "help")
        {
            std::cout << generalHelpMessage << std::endl;
            continue;
        }
        else if (command == "q" || command == "quit")
        {
            std::cout << std::endl << "Bye!" << std::endl;
            return EXIT_SUCCESS;
        }
        else if (command == "l" || command == "list")
        {
            auto users = authenticator.getUsers();
            if (users.empty())
            {
                std::cout << "There are currently no users" << std::endl;
            }
            else
            {
                for (const auto &user : users)
                {
                    std::cout << user.getName() << std::endl;
                } 
            }
            continue;
        }
        else if (command == "a" || command == "add")
        {
            UAuth::User thisUser;
            try
            {
                thisUser = getUserFromJSON();
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
            auto users = authenticator.getUsers();
            for (const auto &user : users)
            {
                if (user.getName() == thisUser.getName())
                {
                    std::cerr << "User: " << user.getName() + " already exists."
                              << "  Consider updating information."
                              << std::endl;
                    continue;
                }
            }
        }
        else
        {
            std::cout << "Unknown command: " << command << std::endl;
        }
        std::cout << generalHelpMessage << std::endl;
    }
    return EXIT_SUCCESS;
}

/// Read the program options from the command line
ProgramOptions parseCommandLineOptions(int argc, char *argv[])
{
    const std::string tableDirectory = std::string(std::getenv("HOME"))
                                     + "/.local/share/UMPS/tables/user.sqlite3";
    ProgramOptions options;
    boost::program_options::options_description desc(
        "uUserTable is a utility for managing users in the user sqlite3 table. Example usage is as follows:\n\n  uUserTable --table user.sqlite3\n\n");
    desc.add_options()
        ("help",       "Produces this help message")
        ("table",
         boost::program_options::value<std::string> ()->default_value(tableDirectory),
         "The filename of the user table");
    boost::program_options::variables_map vm; 
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        options.mError =-1;
        return options;
    }
    if (vm.count("table"))
    {
        options.mUserTable = vm["table"].as<std::string>();
    }
    // Make sure file directory exists
    auto path = std::filesystem::path(options.mUserTable).parent_path();
    if (!path.empty() && !std::filesystem::exists(path))
    {
        if (!std::filesystem::create_directories(path))
        {
            std::cerr << "Failed to create path: " << path << std::endl;
            options.mError = 1;
            return options;
        }
    }
    return options;
}
