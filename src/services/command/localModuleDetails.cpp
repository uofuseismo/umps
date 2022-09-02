#include <string>
#include <filesystem>
#include <unistd.h>
#include <errno.h>
#include <boost/config.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include "umps/services/command/localModuleDetails.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Command;

class LocalModuleDetails::LocalModuleDetailsImpl
{
public:
    LocalModuleDetailsImpl()
    {   
        try
        {
            auto pid = boost::interprocess::ipcdetail::get_current_process_id();
            mProcessIdentifier = static_cast<int64_t> (pid);
        }
        catch (...)
        {
        }
    }   
    void createIPCFileName()
    {
        mIPCFileName = mIPCDirectory / std::filesystem::path {mName + ".ipc"};
    } 
    LocalModuleDetailsImpl& operator=(const LocalModuleDetailsImpl &) = default;
    std::string mName;
    std::filesystem::path mIPCFileName;
    std::filesystem::path mIPCDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    int64_t mProcessIdentifier{0};
    ApplicationStatus mApplicationStatus{ApplicationStatus::Unknown};
};

/// C'tor
LocalModuleDetails::LocalModuleDetails() :
    pImpl(std::make_unique<LocalModuleDetailsImpl> ())
{
}

/// Copy c'tor
LocalModuleDetails::LocalModuleDetails(const LocalModuleDetails &details)
{
    *this = details;
}

/// Move c'tor
LocalModuleDetails::LocalModuleDetails(LocalModuleDetails &&details) noexcept
{
    *this = std::move(details);
}

/// Copy assignment
LocalModuleDetails&
LocalModuleDetails::operator=(const LocalModuleDetails &details)
{
    if (&details == this){return *this;}
    pImpl = std::make_unique<LocalModuleDetailsImpl> (*details.pImpl);
    return *this;
}

/// Move assignment
LocalModuleDetails&
LocalModuleDetails::operator=(LocalModuleDetails &&details) noexcept
{
    if (&details == this){return *this;}
    pImpl = std::move(details.pImpl);
    return *this;
}

/// Reset class
void LocalModuleDetails::clear() noexcept
{
    pImpl = std::make_unique<LocalModuleDetailsImpl> ();
}

/// Destructor
LocalModuleDetails::~LocalModuleDetails() = default;

/// Module name
void LocalModuleDetails::setName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("Name is empty");}
    pImpl->mName = name;
    pImpl->createIPCFileName();
}

std::string LocalModuleDetails::getName() const
{
    if (!haveName()){throw std::runtime_error("Module name not set");}
    return pImpl->mName;
}

bool LocalModuleDetails::haveName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Process ID
void LocalModuleDetails::setProcessIdentifier(
    const int64_t identifier) noexcept
{
    pImpl->mProcessIdentifier = identifier;
}

int64_t LocalModuleDetails::getProcessIdentifier() const noexcept
{
    return pImpl->mProcessIdentifier;
}

/// App status
void LocalModuleDetails::setApplicationStatus(
    const ApplicationStatus status) noexcept
{
    pImpl->mApplicationStatus = status;
}

ApplicationStatus LocalModuleDetails::getApplicationStatus() const noexcept
{
    return pImpl->mApplicationStatus;
}

/// IPC directory
void LocalModuleDetails::setIPCDirectory(const std::string &directory)
{
    pImpl->mIPCDirectory = directory;
    if (isEmpty(directory)){pImpl->mIPCDirectory = "./";}
    if (directory == "."){pImpl->mIPCDirectory = "./";}
    if (haveName()){pImpl->createIPCFileName();}
}

std::string LocalModuleDetails::getIPCDirectory() const noexcept
{
    return pImpl->mIPCDirectory;
}

/// The IPC fil ename
std::string LocalModuleDetails::getIPCFileName() const
{
    if (!haveName()){throw std::runtime_error("Module name not set");}
    return pImpl->mIPCFileName;
}
