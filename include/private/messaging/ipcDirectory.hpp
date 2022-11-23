#ifndef PRIVATE_MESSAGING_IPCDIRECTORY_HPP
#define PRIVATE_MESSAGING_IPCDIRECTORY_HPP
#ifdef UMPS_SRC
#include <filesystem>
#include <string>
namespace
{
/// @brief Creates the IPC directory
void createIPCDirectory(const std::filesystem::path &ipcRootDirectory,
                        UMPS::Logging::ILog *logger)
{
    if (ipcRootDirectory.empty()){return;} // Equivalent to ./
    if (ipcRootDirectory.string() == "."){return;}
    if (ipcRootDirectory.string() == "./"){return;}
    // If the directory doesn't exist then take action
    if (!std::filesystem::exists(ipcRootDirectory))
    {
        std::string debugMessage{"Creating IPC directory: "};
        debugMessage = debugMessage + ipcRootDirectory.string();
        if (logger != nullptr){logger->debug(debugMessage);}
        if (!std::filesystem::create_directories(ipcRootDirectory))
        {
            std::string errorMessage{"Failed to make IPC directory: "};
            errorMessage = errorMessage + ipcRootDirectory.string();
            if (logger != nullptr){logger->error(errorMessage);}
            throw std::runtime_error(errorMessage);
        }
        // Update the permissions
        std::filesystem::permissions(ipcRootDirectory,
                                     std::filesystem::perms::owner_read  |
                                     std::filesystem::perms::owner_write |
                                     std::filesystem::perms::owner_exec);
    }
}
/// @brief Is this an ipc connection?
[[nodiscard]] bool isIPCAddress(const std::string &address)
{
    if (address.empty()){return false;}
    const std::string root{"ipc://"};
    if (address.size() < root.size()){return false;}
    if (address.find(root, 0) == 0){return true;}
    return false;
}
/// @brief Returns the file name from the IPC address
[[nodiscard]] std::filesystem::path ipcAddressToFile(const std::string &address)
{
    std::filesystem::path result;
    if (!isIPCAddress(address)){return result;}
    const std::string root{"ipc://"};
    std::filesystem::path fileName{address.substr(root.size())};
    result = fileName;
    return result;  
}
/// @brief Checks if a connection address is an ipc connection and, if yes,
///        makes the directory.
void createIPCDirectoryFromConnectionString(
    const std::string &address,
    UMPS::Logging::ILog *logger)
{
    auto ipcFileName = ipcAddressToFile(address);
    // String starts with "ipc://" - strip it out and make directory
    if (!ipcFileName.empty())
    {
        auto ipcDirectory = ipcFileName.parent_path();
        createIPCDirectory(ipcDirectory, logger);
    }
}
/// @brief Deletes the IPC file
void removeIPCFile(const std::string &address,
                   UMPS::Logging::ILog *logger)
{
    auto ipcFileName = ipcAddressToFile(address);
    if (!ipcFileName.empty())
    {
        if (std::filesystem::exists(ipcFileName))
        {
            if (logger != nullptr)
            {
                logger->debug("Deleting " + ipcFileName.string());
            }
            if (!std::filesystem::remove(ipcFileName))
            {
                logger->debug("Failed to delete " + ipcFileName.string());
            }
        }
    }
}
}
#endif
#endif
