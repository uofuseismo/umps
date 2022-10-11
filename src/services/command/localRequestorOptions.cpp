#include <chrono>
#include <algorithm>
#include <string>
#include <filesystem>
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/proxyServices/command/remoteRequestorOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Command;

class LocalRequestorOptions::LocalRequestorOptionsImpl
{
public:
    [[nodiscard]] std::string getIPCFileName() const
    {
        if (mModuleName.empty())
        {
            throw std::runtime_error("Module name not set");
        }
        std::filesystem::path fileName{mModuleName + ".ipc"};
        fileName = mIPCDirectory / fileName; 
        return fileName;
    }
    void updateAddress()
    {
        if (!mModuleName.empty() && !mIPCDirectory.empty())
        {
            auto address = "ipc://" + getIPCFileName();
            mOptions.setAddress(address);
        }
    }
    std::string mModuleName;
    std::filesystem::path mIPCDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    UMPS::ProxyServices::Command::RemoteRequestorOptions mOptions;
};

/// C'tor
LocalRequestorOptions::LocalRequestorOptions() :
    pImpl(std::make_unique<LocalRequestorOptionsImpl> ())
{
}

/// Copy c'tor
LocalRequestorOptions::LocalRequestorOptions(
    const LocalRequestorOptions &options)
{
    *this = options;
}

/// Copy m'tor
LocalRequestorOptions::LocalRequestorOptions(
    LocalRequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
LocalRequestorOptions&
LocalRequestorOptions::operator=(const LocalRequestorOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<LocalRequestorOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
LocalRequestorOptions&
LocalRequestorOptions::operator=(LocalRequestorOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
LocalRequestorOptions::~LocalRequestorOptions() = default;

/// Reset
void LocalRequestorOptions::clear() noexcept
{
    pImpl = std::make_unique<LocalRequestorOptionsImpl> ();
} 

/// Module name
void LocalRequestorOptions::setModuleName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("Module name is empty");}
    pImpl->mModuleName = name;
    std::remove_if(pImpl->mModuleName.begin(),
                   pImpl->mModuleName.end(), ::isspace);
    pImpl->updateAddress();
}

std::string LocalRequestorOptions::getModuleName() const
{
    if (!haveModuleName()){throw std::runtime_error("Module name not set");}
    return pImpl->mModuleName;
}

bool LocalRequestorOptions::haveModuleName() const noexcept
{
    return !pImpl->mModuleName.empty();
}

/// IPC directory
void LocalRequestorOptions::setIPCDirectory(const std::string &directory)
{
    if (directory.empty())
    {
        pImpl->mIPCDirectory = "./";
    }
    else
    {
        if (!std::filesystem::exists(directory))
        {
            throw std::invalid_argument("IPC directory: " + directory
                                      + " does not exist");
        }
        pImpl->mIPCDirectory = directory;
    }
    pImpl->updateAddress();
}

std::string LocalRequestorOptions::getIPCDirectory() const noexcept
{
    return pImpl->mIPCDirectory;
}

std::string LocalRequestorOptions::getIPCFileName() const
{
    return pImpl->getIPCFileName();
    //auto moduleName = getModuleName(); // Throws
    //std::filesystem::path fileName{moduleName + ".ipc"};
    //fileName = std::filesystem::path{getIPCDirectory()} / fileName; 
    //return fileName.string();
}

/// Time-out
void LocalRequestorOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut)
{
    pImpl->mOptions.setReceiveTimeOut(timeOut);
} 

/// Requestor options
UMPS::ProxyServices::Command::RemoteRequestorOptions
    LocalRequestorOptions::getOptions() const
{
    return pImpl->mOptions;
}
