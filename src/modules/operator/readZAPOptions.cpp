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

/// Loads the client ZAP options
UAuth::ZAPOptions UMPS::Modules::Operator::readZAPClientOptions(
    const std::string &iniFile)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Load the ZAP options
    return readZAPClientOptions(propertyTree);
}

/// Loads the ZAP client options
UAuth::ZAPOptions UMPS::Modules::Operator::readZAPClientOptions(
    const boost::property_tree::ptree &propertyTree)
{
    UAuth::ZAPOptions zapOptions;
    auto securityLevel = static_cast<UAuth::SecurityLevel>
       (propertyTree.get<int> ("uOperator.securityLevel",
                    static_cast<int> (zapOptions.getSecurityLevel())));
    if (static_cast<int> (securityLevel) < 0 ||
        static_cast<int> (securityLevel) > 4)
    {
        throw std::invalid_argument("Security level must be in range [0,4]");
    }
    // Define ZAP options
    zapOptions.setGrasslandsClient();
    if (securityLevel == UAuth::SecurityLevel::GRASSLANDS)
    {   
        zapOptions.setGrasslandsClient();
    }   
    else if (securityLevel == UAuth::SecurityLevel::STRAWHOUSE)
    {   
        zapOptions.setStrawhouseClient();
    }   
    else if (securityLevel == UAuth::SecurityLevel::WOODHOUSE)
    {   
        UAuth::Certificate::UserNameAndPassword credentials;
        std::string userName{std::getenv("UMPS_USER")};
        std::string password{std::getenv("UMPS_PASSWORD")}; 
        userName = propertyTree.get<std::string> ("uOperator.clientUserName",
                                                  userName);
        password = propertyTree.get<std::string> ("uOperator.clientPassword",
                                                  password);
        credentials.setUserName(userName);
        credentials.setPassword(password);
        zapOptions.setWoodhouseClient(credentials);
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
        zapOptions.setStonehouseClient(serverKeys, clientKeys);
    }   
    else
    {   
        throw std::runtime_error("Unhandled security level: "
                            + std::to_string(static_cast<int> (securityLevel)));
    }
    return zapOptions;
}

///--------------------------------------------------------------------------///
///                            Server ZAP Options                            ///
///--------------------------------------------------------------------------///
/// Loads the server ZAP options
UAuth::ZAPOptions UMPS::Modules::Operator::readZAPServerOptions(
    const std::string &iniFile)
{
    if (!std::filesystem::exists(iniFile))
    {   
        throw std::invalid_argument("Initialiation file: " + iniFile
                                  + " does not exist");
    }   
    // Parse the initialization file
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Load the ZAP options
    return readZAPServerOptions(propertyTree);
}

/// Loads the server ZAP options 
UAuth::ZAPOptions UMPS::Modules::Operator::readZAPServerOptions(
    const boost::property_tree::ptree &propertyTree)
{
    UAuth::ZAPOptions zapOptions;
    auto securityLevel = static_cast<UAuth::SecurityLevel>
       (propertyTree.get<int> ("uOperator.securityLevel",
                    static_cast<int> (zapOptions.getSecurityLevel())));
    if (static_cast<int> (securityLevel) < 0 ||
        static_cast<int> (securityLevel) > 4)
    {
        throw std::invalid_argument("Security level must be in range [0,4]");
    }
    zapOptions.setGrasslandsServer();
    if (securityLevel == UAuth::SecurityLevel::GRASSLANDS)
    {
        zapOptions.setGrasslandsServer();
    }
    else if (securityLevel == UAuth::SecurityLevel::STRAWHOUSE)
    {
        zapOptions.setStrawhouseServer();
    }
    else if (securityLevel == UAuth::SecurityLevel::WOODHOUSE)
    {
        zapOptions.setWoodhouseServer();
    }
    else if (securityLevel == UAuth::SecurityLevel::STONEHOUSE)
    {
        auto publicKeyFile
            = propertyTree.get<std::string> ("uOperator.serverPublicKeyFile");
        auto privateKeyFile
            = propertyTree.get<std::string> ("uOperator.serverPrivateKeyFile");
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
