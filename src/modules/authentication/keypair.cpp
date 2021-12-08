#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/messaging/authentication/certificate/keys.hpp"

struct ProgramOptions
{
    std::string mPublicKeyFile = "publicKey.key";
    std::string mPrivateKeyFile = "privateKey.key";
    std::string mMetadata = "";
    int mError = 0;
};

ProgramOptions parseCommandLineOptions(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    auto options = parseCommandLineOptions(argc, argv);
    if (options.mError != 0)
    {
        if (options.mError == -1){return EXIT_SUCCESS;}
        std::cerr << "Execution failed" << std::endl; 
        return EXIT_FAILURE;
    }
    auto publicKeyFileName = options.mPublicKeyFile;
    auto privateKeyFileName = options.mPrivateKeyFile;
    UMPS::Messaging::Authentication::Certificate::Keys certificate;
    certificate.create();
    certificate.setMetadata(options.mMetadata);
    certificate.writePublicKeyToTextFile(publicKeyFileName);
    certificate.writePrivateKeyToTextFile(privateKeyFileName);
    return EXIT_SUCCESS;
}

///--------------------------------------------------------------------------///
///                            Utility Functions                             ///
///--------------------------------------------------------------------------///
/// Read the program options from the command line
ProgramOptions parseCommandLineOptions(int argc, char *argv[])
{
    ProgramOptions options;
    boost::program_options::options_description desc(
R""""(uKeyPair is a utility for generating a public/private keypair for use in UMPS.
Example usage is as follows:

    uKeyPair --publickey publickey.txt --privatekey privatekey.txt --keyname test_key

Allowed options)"""");
    desc.add_options()
        ("help",       "Produces this help message.")
        ("publickey",  
         boost::program_options::value<std::string> (),
         "The name of the public key file.")
        ("privatekey",
         boost::program_options::value<std::string> (),
         "The name of the private key file.")
        ("keyname",
         boost::program_options::value<std::string> (),
         "Text descriptor for you key.  This will be embedded in the file itself.");
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
    if (vm.count("publickey"))
    {
        options.mPublicKeyFile = vm["publickey"].as<std::string>();
    }
    if (vm.count("privatekey"))
    {
        options.mPrivateKeyFile = vm["privatekey"].as<std::string>();
    }
    if (vm.count("keyname"))
    {
        options.mMetadata = vm["keyname"].as<std::string> ();
    }
    if (options.mPublicKeyFile == options.mPrivateKeyFile)
    {
        std::cerr << "ERROR - public key file matches private key file"
                  << std::endl;
        options.mError = 1; 
    }
    if (options.mPublicKeyFile.empty())
    {
        std::cerr << "Public key file is empty" << std::endl;
        options.mError = 1;
        return options;
    }
    if (options.mPrivateKeyFile.empty())
    {
        std::cerr << "Private key file is empty" << std::endl;
        options.mError = 1;
        return options;
    }
    // Make sure file directory exists
    auto path1 = std::filesystem::path(options.mPublicKeyFile).parent_path();
    auto path2 = std::filesystem::path(options.mPrivateKeyFile).parent_path();
    if (!path1.empty() && !std::filesystem::exists(path1))
    {
        std::filesystem::create_directories(path1);
        if (!std::filesystem::exists(path1))
        {
            std::cerr << "Failed to create path: " << path1 << std::endl;
            options.mError = 1;
            return options;
        }
    }
    if (!path2.empty() && !std::filesystem::exists(path2))
    {
        std::filesystem::create_directories(path2);
        if (!std::filesystem::exists(path2))
        {
            std::cerr << "Failed to create path: " << path2 << std::endl;
            options.mError = 1;
            return options;
        }
    }
    // Warn user
    if (std::filesystem::exists(options.mPublicKeyFile))
    {
        std::cerr << "Warning - will overwrite: "
                  << options.mPublicKeyFile << std::endl;
    }
    if (std::filesystem::exists(options.mPrivateKeyFile))
    {
        std::cerr << "Warning - will overwrite: "
                  << options.mPrivateKeyFile << std::endl;
    }
    return options;
}
