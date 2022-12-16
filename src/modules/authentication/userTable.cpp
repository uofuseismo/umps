#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>
#include "umps/authentication/sqlite3Authenticator.hpp"
#include "umps/authentication/user.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"

namespace UAuth = UMPS::Authentication;

enum class Job
{
    None = 0,
    Add = 1,
    Update = 2,
    Delete = 3
};

struct ProgramOptions
{
    std::string mUserTable;
    std::string mJSONFile;
    Job mJob{Job::None};
    int mError{0};
};

ProgramOptions parseCommandLineOptions(int argc, char *argv[]);

[[nodiscard]] UAuth::User loadUserFromJSONFile(const std::string &fileName)
{
    UAuth::User user;
    std::ifstream inFile(fileName);
    nlohmann::json obj;
    inFile >> obj;
    auto name = obj["name"].get<std::string> ();
    user.setName(name); // Throws
    auto email = obj["email"].get<std::string> ();
    user.setEmail(email); // Throws
    if (!obj["password"].is_null())
    {
        auto password = obj["password"].get<std::string> (); 
        if (!password.empty())
        {

        }
    }
    if (!obj["publickey_file"].is_null())
    {
        auto publicKeyFileName = obj["publickey_file"].get<std::string> ();
        UAuth::Certificate::Keys key; 
        key.loadFromTextFile(publicKeyFileName);
        user.setPublicKey( std::string{key.getPublicTextKey().data()} );
    }
    auto privileges = UAuth::UserPrivileges::ReadOnly;
    if (!obj["privileges"].is_null())
    {
        auto textPrivileges = obj["privileges"].get<std::string> ();
        if (textPrivileges.find("admin") != std::string::npos)
        {
            privileges = UAuth::UserPrivileges::Administrator;
        }
        else if (textPrivileges.find("read_write") != std::string::npos)
        {
            privileges = UAuth::UserPrivileges::ReadWrite;
        }
        else if (textPrivileges.find("read_only") != std::string::npos)
        {
            privileges = UAuth::UserPrivileges::ReadOnly; 
        }
        else
        {
            std::cout << "Unknown privileges: " << textPrivileges << std::endl; 
            std::cout << "Use: read_only, read_write, admin" << std::endl;
        }
    }
    user.setPrivileges(privileges);
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

/*
std::string generalHelpMessage = R"""(
Available commands:

   (l)ist    Lists the users in the database.
   (a)dd     Adds a user.
   (u)pdate  Updates the users.
   (q)uit    Exits the program.
   (h)elp    Displays this message.
)""";
*/

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
    UAuth::User user;
    try
    {
        user = loadUserFromJSONFile(options.mJSONFile);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // 
    if (options.mJob == Job::Add)
    {
        std::cout << "Attempting to add: " << std::endl << user << std::endl;
        authenticator.addUser(user);
    }
    else if (options.mJob == Job::Update)
    {
        std::cout << "Attempting to update: " << std::endl << user << std::endl;
        authenticator.updateUser(user);
    }
    else if (options.mJob == Job::Delete)
    {
        std::cout << "Attempting to delete: " << std::endl << user << std::endl;
        authenticator.deleteUser(user);
    }
    else
    {
        std::cerr << "Unhandled job" << std::endl;
        return EXIT_FAILURE;
    }
/*
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
*/
    return EXIT_SUCCESS;
}

/// Read the program options from the command line
ProgramOptions parseCommandLineOptions(int argc, char *argv[])
{
    const std::string tableDirectory = std::string(std::getenv("HOME"))
                                     + "/.local/share/UMPS/tables/user.sqlite3";
    ProgramOptions options;
    boost::program_options::options_description desc(
R""""(
uUserTable is a utility for managing users in the user sqlite3 table. Example usage is as follows:

   uUserTable --add user.json --table user.sqlite3

A valid JSON table looks like:

{
    "name" : "user",
    "email" : "user@domain.com",
    "password" : null,
    "publickey_file" : "/path/to/publicKey.txt",
    "privileges" : "read_write"
}

Here privileges could also be read_only or admin.  The password is null otherwise, it would be a
character string, say, password : "password".  To delete a user you really only need to provide
the user name and email address.

Command options are as follows)"""");
    desc.add_options()
        ("help,h",
         "Produces this help message")
        ("add,a",
         boost::program_options::value<std::string> (),
         "Adds a user in the whose information is specified in the given JSON file to the user table")
        ("update,u",
         boost::program_options::value<std::string> (),
         "Updates a user whose new information is specified in the given JSON file in the user table")
        ("delete,d",
         boost::program_options::value<std::string> (),
        "Deletes a user whose information is specified in the given JSON file from the user table")
        ("table,t",
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
    if (vm.count("add"))
    {
        options.mJSONFile = vm["add"].as<std::string> ();
        options.mJob = Job::Add; 
    }
    else if (vm.count("update"))
    {
        options.mJSONFile = vm["update"].as<std::string> ();
        options.mJob = Job::Update;
    }
    else if (vm.count("delete"))
    {
        options.mJSONFile = vm["delete"].as<std::string> ();
        options.mJob = Job::Delete;
    }
    else
    {
        std::cerr << "No action specified" << std::endl;
        options.mJob = Job::None;
        options.mError = 1;
        return options;
    }
    // Ensure the user's JSON file exists
    if (options.mJob != Job::None)
    {
        if (!std::filesystem::exists(options.mJSONFile))
        {
            std::cerr << "Failed to open user JSON file: "
                      << options.mJSONFile << std::endl;
            options.mError = 1;
            return options;
        }
    }
    // The sqlite3 table
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
