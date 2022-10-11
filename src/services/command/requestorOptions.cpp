#include <chrono>
#include <algorithm>
#include <string>
#include <filesystem>
#include "umps/services/command/requestorOptions.hpp"
#include "umps/proxyServices/command/requestorOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Command;

class RequestorOptions::RequestorOptionsImpl
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
    UMPS::ProxyServices::Command::RequestorOptions mOptions;
};

/// C'tor
RequestorOptions::RequestorOptions() :
    pImpl(std::make_unique<RequestorOptionsImpl> ())
{
}

/// Copy c'tor
RequestorOptions::RequestorOptions(
    const RequestorOptions &options)
{
    *this = options;
}

/// Copy m'tor
RequestorOptions::RequestorOptions(
    RequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RequestorOptions&
RequestorOptions::operator=(const RequestorOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RequestorOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RequestorOptions&
RequestorOptions::operator=(RequestorOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
RequestorOptions::~RequestorOptions() = default;

/// Reset
void RequestorOptions::clear() noexcept
{
    pImpl = std::make_unique<RequestorOptionsImpl> ();
} 

/// Module name
void RequestorOptions::setModuleName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("Module name is empty");}
    pImpl->mModuleName = name;
    std::remove_if(pImpl->mModuleName.begin(),
                   pImpl->mModuleName.end(), ::isspace);
    pImpl->updateAddress();
}

std::string RequestorOptions::getModuleName() const
{
    if (!haveModuleName()){throw std::runtime_error("Module name not set");}
    return pImpl->mModuleName;
}

bool RequestorOptions::haveModuleName() const noexcept
{
    return !pImpl->mModuleName.empty();
}

/// IPC directory
void RequestorOptions::setIPCDirectory(const std::string &directory)
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

std::string RequestorOptions::getIPCDirectory() const noexcept
{
    return pImpl->mIPCDirectory;
}

std::string RequestorOptions::getIPCFileName() const
{
    return pImpl->getIPCFileName();
    //auto moduleName = getModuleName(); // Throws
    //std::filesystem::path fileName{moduleName + ".ipc"};
    //fileName = std::filesystem::path{getIPCDirectory()} / fileName; 
    //return fileName.string();
}

/// Time-out
void RequestorOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut)
{
    pImpl->mOptions.setReceiveTimeOut(timeOut);
} 

/// Requestor options
UMPS::ProxyServices::Command::RequestorOptions
    RequestorOptions::getOptions() const
{
    return pImpl->mOptions;
}
