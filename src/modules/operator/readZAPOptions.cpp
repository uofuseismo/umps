#include <string>
#include <filesystem>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/modules/operator/readZAPOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"

namespace UAuth = UMPS::Authentication;

namespace
{

UAuth::ZAPOptions readZAPClientOptions(
    const boost::property_tree::ptree &propertyTree,
    const std::string &section = "uOperator")
{
    UAuth::ZAPOptions zapOptions;
    auto securityLevel = static_cast<UAuth::SecurityLevel>
       (propertyTree.get<int> (section + ".securityLevel",
                    static_cast<int> (zapOptions.getSecurityLevel())));
    if (static_cast<int> (securityLevel) < 0 ||
        static_cast<int> (securityLevel) > 4)
    {   
        throw std::invalid_argument("Security level must be in range [0,4]");
    }   
    // Define ZAP options
    zapOptions.setGrasslandsClient();
    if (securityLevel == UAuth::SecurityLevel::Grasslands)
    {   
        zapOptions.setGrasslandsClient();
    }   
    else if (securityLevel == UAuth::SecurityLevel::Strawhouse)
    {   
        zapOptions.setStrawhouseClient();
    }   
    else if (securityLevel == UAuth::SecurityLevel::Woodhouse)
    {   
        UAuth::Certificate::UserNameAndPassword credentials;
        std::string userName;
        const char *userPtr = std::getenv("UMPS_USER");
        if (userPtr != nullptr) 
        {
            if (std::strlen(userPtr) > 0)
            {
                userName = std::string {userPtr};
            }
        }
        std::string password;
        const char *passwordPtr = std::getenv("UMPS_PASSWORD");
        if (passwordPtr != nullptr)
        {
            if (std::strlen(passwordPtr) > 0)
            {
                password = std::string {passwordPtr};
            }
        }
        //std::string userName{std::getenv("UMPS_USER")};
        //std::string password{std::getenv("UMPS_PASSWORD")}; 
        userName = propertyTree.get<std::string> (section + ".clientUserName",
                                                  userName);
        password = propertyTree.get<std::string> (section + ".clientPassword",
                                                  password);
        credentials.setUserName(userName);
        credentials.setPassword(password);
        zapOptions.setWoodhouseClient(credentials);
    }   
    else if (securityLevel == UAuth::SecurityLevel::Stonehouse)
    {   
        auto serverPublicKeyFile
            = propertyTree.get<std::string> (section + ".serverPublicKeyFile");
        auto clientPublicKeyFile
            = propertyTree.get<std::string> (section + ".clientPublicKeyFile");
        auto clientPrivateKeyFile
            = propertyTree.get<std::string> (section + ".clientPrivateKeyFile");
        UAuth::Certificate::Keys serverKeys, clientKeys;
        serverKeys.loadFromTextFile(serverPublicKeyFile);
        clientKeys.loadFromTextFile(clientPublicKeyFile);
        clientKeys.loadFromTextFile(clientPrivateKeyFile);
        zapOptions.setStonehouseClient(serverKeys, clientKeys);
    }   
    else
    {   
        throw std::runtime_error("Unhandled security level: "
                            + std::to_string(static_cast<int> (securityLevel)));
    }   
    return zapOptions;
}

/// Loads the server ZAP options 
UAuth::ZAPOptions readZAPServerOptions(
    const boost::property_tree::ptree &propertyTree,
    const std::string &section = "uOperator")
{
    UAuth::ZAPOptions zapOptions;
    auto securityLevel = static_cast<UAuth::SecurityLevel>
       (propertyTree.get<int> (section + ".securityLevel",
                    static_cast<int> (zapOptions.getSecurityLevel())));
    if (static_cast<int> (securityLevel) < 0 ||
        static_cast<int> (securityLevel) > 4)
    {
        throw std::invalid_argument("Security level must be in range [0,4]");
    }
    zapOptions.setGrasslandsServer();
    if (securityLevel == UAuth::SecurityLevel::Grasslands)
    {
        zapOptions.setGrasslandsServer();
    }
    else if (securityLevel == UAuth::SecurityLevel::Strawhouse)
    {
        zapOptions.setStrawhouseServer();
    }
    else if (securityLevel == UAuth::SecurityLevel::Woodhouse)
    {
        zapOptions.setWoodhouseServer();
    }
    else if (securityLevel == UAuth::SecurityLevel::Stonehouse)
    {
        auto publicKeyFile
            = propertyTree.get<std::string> (section + ".serverPublicKeyFile");
        auto privateKeyFile
            = propertyTree.get<std::string> (section + ".serverPrivateKeyFile");
        UAuth::Certificate::Keys serverKeys;
        serverKeys.loadFromTextFile(publicKeyFile);
        serverKeys.loadFromTextFile(privateKeyFile);
        zapOptions.setStonehouseServer(serverKeys);
    }
    else
    {
        throw std::runtime_error("Unhandled security level: " 
                            + std::to_string(static_cast<int> (securityLevel)));
    }
    return zapOptions;
}

}

/// Loads the client ZAP options
UAuth::ZAPOptions UMPS::Modules::Operator::readZAPClientOptions(
    const std::string &iniFile, const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }
    if (section.empty())
    {
        throw std::invalid_argument("Section is empty");
    }
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Load the ZAP options
    return ::readZAPClientOptions(propertyTree, section);
}

/// Loads the server ZAP options
UAuth::ZAPOptions UMPS::Modules::Operator::readZAPServerOptions(
    const std::string &iniFile, const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {   
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }   
    if (section.empty())
    {
        throw std::invalid_argument("Section is empty");
    }
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Load the ZAP options
    return ::readZAPServerOptions(propertyTree, section);
}
