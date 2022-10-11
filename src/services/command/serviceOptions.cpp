#include <filesystem>
#include "umps/services/command/serviceOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Command;

class ServiceOptions::ServiceOptionsImpl
{
public:
    std::string mModuleName;
    std::filesystem::path mIPCDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    std::filesystem::path mModuleTable
        = std::filesystem::path{ std::string(std::getenv("HOME")) }
        / std::filesystem::path{".local/share/UMPS/tables/localModuleTable.sqlite3"};
    std::function<
        std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *contents,
         const size_t length)
    > mCallback;
    bool mHaveCallback = false;
};

/// C'tor
ServiceOptions::ServiceOptions() :
    pImpl(std::make_unique<ServiceOptionsImpl> ())
{
}

/// Copy c'tor
ServiceOptions::ServiceOptions(const ServiceOptions &options)
{
    *this = options;
}

/// Move c'tor
ServiceOptions::ServiceOptions(ServiceOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ServiceOptions&
ServiceOptions::operator=(const ServiceOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ServiceOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ServiceOptions&
ServiceOptions::operator=(ServiceOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Module name
void ServiceOptions::setModuleName(const std::string &moduleName)
{
    if (::isEmpty(moduleName))
    {
        throw std::invalid_argument("Module name is empty");
    }
    pImpl->mModuleName = moduleName;
}

std::string ServiceOptions::getModuleName() const
{
    if (!haveModuleName()){throw std::runtime_error("Module name not set");}
    return pImpl->mModuleName;
}

bool ServiceOptions::haveModuleName() const noexcept
{
    return !pImpl->mModuleName.empty();
}

std::string ServiceOptions::getAddress() const
{
    if (!haveModuleName())
    {
        throw std::runtime_error("Module name not set");
    }
    auto fileName = pImpl->mIPCDirectory 
                  / std::filesystem::path {getModuleName() + ".ipc"};
    auto address = "ipc://" + fileName.string();
    return address;
}

/// Sets the callback
void ServiceOptions::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mCallback = callback;
    pImpl->mHaveCallback = true;
}

std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
    ServiceOptions::getCallback() const
{
    if (!haveCallback())
    {
        throw std::runtime_error("Callback not set");
    }
    return pImpl->mCallback;
}

bool ServiceOptions::haveCallback() const noexcept
{
    return pImpl->mHaveCallback;
}

/// IPC directory
void ServiceOptions::setIPCDirectory(const std::string &directory)
{
    pImpl->mIPCDirectory = directory;
    if (isEmpty(directory)){pImpl->mIPCDirectory = "./";}
    if (directory == "."){pImpl->mIPCDirectory = "./";}
}

std::string ServiceOptions::getIPCDirectory() const noexcept
{
    return pImpl->mIPCDirectory;
}

/// Module table file
void ServiceOptions::setModuleTable(const std::string &fileName)
{
    if (std::filesystem::exists(fileName))
    {
        pImpl->mModuleTable = fileName;
        return;
    }
    // Ensure directory exists
    std::filesystem::path path{fileName};
    auto parentPath = path.parent_path();
    if (parentPath.empty())
    {
        pImpl->mModuleTable = "./" + fileName;
        return;
    }
    if (!std::filesystem::create_directories(parentPath))
    {
        throw std::runtime_error("Failed to create directory: " + fileName);
    }
}

std::string ServiceOptions::getModuleTable() const noexcept
{
    return pImpl->mModuleTable;
}

/// Reset class
void ServiceOptions::clear() noexcept
{
    pImpl = std::make_unique<ServiceOptionsImpl> ();
}

/// Destructor
ServiceOptions::~ServiceOptions() = default;
