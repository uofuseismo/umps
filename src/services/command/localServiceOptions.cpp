#include <filesystem>
#include "umps/services/command/localServiceOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Command;

class LocalServiceOptions::LocalServiceOptionsImpl
{
public:
    std::string mModuleName;
    std::filesystem::path mIPCDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    std::function<
        std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *contents,
         const size_t length)
    > mCallback;
    bool mHaveCallback = false;
};

/// C'tor
LocalServiceOptions::LocalServiceOptions() :
    pImpl(std::make_unique<LocalServiceOptionsImpl> ())
{
}

/// Copy c'tor
LocalServiceOptions::LocalServiceOptions(const LocalServiceOptions &options)
{
    *this = options;
}

/// Move c'tor
LocalServiceOptions::LocalServiceOptions(LocalServiceOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
LocalServiceOptions&
LocalServiceOptions::operator=(const LocalServiceOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<LocalServiceOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
LocalServiceOptions&
LocalServiceOptions::operator=(LocalServiceOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Module name
void LocalServiceOptions::setModuleName(const std::string &moduleName)
{
    if (::isEmpty(moduleName))
    {
        throw std::invalid_argument("Module name is empty");
    }
    pImpl->mModuleName = moduleName;
}

std::string LocalServiceOptions::getModuleName() const
{
    if (!haveModuleName()){throw std::runtime_error("Module name not set");}
    return pImpl->mModuleName;
}

bool LocalServiceOptions::haveModuleName() const noexcept
{
    return !pImpl->mModuleName.empty();
}

std::string LocalServiceOptions::getAddress() const
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
void LocalServiceOptions::setCallback(
    const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
                        &callback)
{
    pImpl->mCallback = callback;
    pImpl->mHaveCallback = true;
}

std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                        (const std::string &, const void *, size_t)>
    LocalServiceOptions::getCallback() const
{
    if (!haveCallback())
    {
        throw std::runtime_error("Callback not set");
    }
    return pImpl->mCallback;
}

bool LocalServiceOptions::haveCallback() const noexcept
{
    return pImpl->mHaveCallback;
}

/// IPC directory
void LocalServiceOptions::setIPCDirectory(const std::string &directory)
{
    pImpl->mIPCDirectory = directory;
    if (isEmpty(directory)){pImpl->mIPCDirectory = "./";}
    if (directory == "."){pImpl->mIPCDirectory = "./";}
}

std::string LocalServiceOptions::getIPCDirectory() const noexcept
{
    return pImpl->mIPCDirectory;
}

/// Reset class
void LocalServiceOptions::clear() noexcept
{
    pImpl = std::make_unique<LocalServiceOptionsImpl> ();
}

/// Destructor
LocalServiceOptions::~LocalServiceOptions() = default;
