#include <string>
#include <filesystem>
#include <unistd.h>
#include <errno.h>
#include <boost/config.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include "umps/services/command/moduleDetails.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Command;

class ModuleDetails::ModuleDetailsImpl
{
public:
    ModuleDetailsImpl()
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
    ModuleDetailsImpl& operator=(const ModuleDetailsImpl &) = default;
    std::string mName;
    std::filesystem::path mIPCFileName;
    std::filesystem::path mIPCDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    int64_t mProcessIdentifier{0};
    ModuleDetails::ApplicationStatus
        mApplicationStatus{ModuleDetails::ApplicationStatus::Unknown};
};

/// C'tor
ModuleDetails::ModuleDetails() :
    pImpl(std::make_unique<ModuleDetailsImpl> ())
{
}

/// Copy c'tor
ModuleDetails::ModuleDetails(const ModuleDetails &details)
{
    *this = details;
}

/// Move c'tor
ModuleDetails::ModuleDetails(ModuleDetails &&details) noexcept
{
    *this = std::move(details);
}

/// Copy assignment
ModuleDetails&
ModuleDetails::operator=(const ModuleDetails &details)
{
    if (&details == this){return *this;}
    pImpl = std::make_unique<ModuleDetailsImpl> (*details.pImpl);
    return *this;
}

/// Move assignment
ModuleDetails&
ModuleDetails::operator=(ModuleDetails &&details) noexcept
{
    if (&details == this){return *this;}
    pImpl = std::move(details.pImpl);
    return *this;
}

/// Reset class
void ModuleDetails::clear() noexcept
{
    pImpl = std::make_unique<ModuleDetailsImpl> ();
}

/// Destructor
ModuleDetails::~ModuleDetails() = default;

/// Module name
void ModuleDetails::setName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("Name is empty");}
    pImpl->mName = name;
    pImpl->createIPCFileName();
}

std::string ModuleDetails::getName() const
{
    if (!haveName()){throw std::runtime_error("Module name not set");}
    return pImpl->mName;
}

bool ModuleDetails::haveName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Process ID
void ModuleDetails::setProcessIdentifier(
    const int64_t identifier) noexcept
{
    pImpl->mProcessIdentifier = identifier;
}

int64_t ModuleDetails::getProcessIdentifier() const noexcept
{
    return pImpl->mProcessIdentifier;
}

/// App status
void ModuleDetails::setApplicationStatus(
    const ModuleDetails::ApplicationStatus status) noexcept
{
    pImpl->mApplicationStatus = status;
}

ModuleDetails::ApplicationStatus
ModuleDetails::getApplicationStatus() const noexcept
{
    return pImpl->mApplicationStatus;
}

/// IPC directory
void ModuleDetails::setIPCDirectory(const std::string &directory)
{
    pImpl->mIPCDirectory = directory;
    if (isEmpty(directory)){pImpl->mIPCDirectory = "./";}
    if (directory == "."){pImpl->mIPCDirectory = "./";}
    if (haveName()){pImpl->createIPCFileName();}
}

std::string ModuleDetails::getIPCDirectory() const noexcept
{
    return pImpl->mIPCDirectory;
}

/// The IPC fil ename
std::string ModuleDetails::getIPCFileName() const
{
    if (!haveName()){throw std::runtime_error("Module name not set");}
    return pImpl->mIPCFileName;
}
