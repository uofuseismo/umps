#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <boost/asio/ip/host_name.hpp>
#include <boost/interprocess/detail/os_thread_functions.hpp>
#include <nlohmann/json.hpp>
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Command;

class ModuleDetails::ModuleDetailsImpl
{
public:
    ModuleDetailsImpl()
    {   
        try
        {
            mMachine = boost::asio::ip::host_name();
        }
        catch (...)
        {
        }
        try
        {
            mProcessIdentifier = static_cast<int64_t> (getpid());
        }
        catch (...)
        {
        }
        try
        {
            mParentProcessIdentifier = static_cast<int64_t> (getppid());
        }
        catch (...)
        {
        }
        try
        {
           mExecutable = std::string{program_invocation_name};
        }
        catch (...)
        {
        }
    }   
    ModuleDetailsImpl& operator=(const ModuleDetailsImpl &) = default;
    std::string mExecutable;
    std::string mName;
    std::string mMachine;
    int64_t mProcessIdentifier{0};
    int64_t mParentProcessIdentifier{0};
    uint16_t mInstance{0};
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
ModuleDetails& ModuleDetails::operator=(const ModuleDetails &details)
{
    if (&details == this){return *this;}
    pImpl = std::make_unique<ModuleDetailsImpl> (*details.pImpl);
    return *this;
}

/// Move assignment
ModuleDetails& ModuleDetails::operator=(ModuleDetails &&details) noexcept
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

/// Instance
void ModuleDetails::setInstance(const uint16_t instance) noexcept
{
    pImpl->mInstance = instance;
}

uint16_t ModuleDetails::getInstance() const noexcept
{
    return pImpl->mInstance;
}

/// Machine
void ModuleDetails::setMachine(const std::string &machine) noexcept
{
    pImpl->mMachine = machine;
}

std::string ModuleDetails::getMachine() const noexcept
{
    return pImpl->mMachine;
}

/// Parent ID
void ModuleDetails::setParentProcessIdentifier(
    const int64_t identifier) noexcept
{
    pImpl->mParentProcessIdentifier = identifier;
}

int64_t ModuleDetails::getParentProcessIdentifier() const noexcept
{
    return pImpl->mParentProcessIdentifier;
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

/// Executable name
void ModuleDetails::setExecutableName(const std::string &name) noexcept
{
    pImpl->mExecutable = name;
}

std::string ModuleDetails::getExecutableName() const noexcept
{
    return pImpl->mExecutable;
}

/// Print out the module details
std::ostream&
UMPS::ProxyServices::Command::operator<<(std::ostream &os,
                                         const ModuleDetails &details)
{
    std::stringstream result;
    result << "Module Details:" << std::endl;
    if (details.haveName())
    {
        result << "  Name: " << details.getName() << std::endl;
    }
    else
    {
        result << "  Name: Undefined" << std::endl;
    }
    result << "  Executable: " << details.getExecutableName() << std::endl;
    result << "  Process Identifier: "
           << details.getProcessIdentifier() << std::endl;
    result << "  Parent Process Identifier: "
           << details.getParentProcessIdentifier() << std::endl;
    return os << result.str();
}
