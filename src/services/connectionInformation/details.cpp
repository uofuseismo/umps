#include <nlohmann/json.hpp>
#include "umps/services/connectionInformation/details.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Messaging::Authentication;

class Details::DetailsImpl
{
public:
    std::string mName;
    std::string mConnectionString;
    UAuth::SecurityLevel mSecurityLevel = UAuth::SecurityLevel::GRASSLANDS; 
};

/// C'tor
Details::Details() :
    pImpl(std::make_unique<DetailsImpl> ())
{
}

/// Copy c'tor
Details::Details(const Details &details)
{
    *this = details;
}

/// Move c'tor
Details::Details(Details &&details) noexcept
{
    *this = std::move(details);
}

/// Copy assignment
Details& Details::operator=(const Details &details)
{
    if (&details == this){return *this;}
    pImpl = std::make_unique<DetailsImpl> (*details.pImpl);
    return *this;
}

/// Move assignment
Details& Details::operator=(Details &&details) noexcept
{
    if (&details == this){return *this;}
    pImpl = std::move(details.pImpl);
    return *this;
}

/// Destructor
Details::~Details() = default;

/// Reset class
void Details::clear() noexcept
{
    pImpl = std::make_unique<DetailsImpl> ();
}

/// Name
void Details::setName(const std::string &name)
{
    if (isEmpty(name))
    {
        throw std::invalid_argument("Name is empty");
    }
    pImpl->mName = name;
}

std::string Details::getName() const
{
    if (!haveName()){throw std::runtime_error("Name not set");}
    return pImpl->mName;
}

bool Details::haveName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Connection string
void Details::setConnectionString(const std::string &connectionString)
{
    if (isEmpty(connectionString))
    {
        throw std::invalid_argument("Connection string is empty");
    }
    pImpl->mConnectionString = connectionString;
}

std::string Details::getConnectionString() const
{
    if (!haveConnectionString())
    {
        throw std::runtime_error("Connection string not defined");
    }
    return pImpl->mConnectionString;
}

bool Details::haveConnectionString() const noexcept
{
    return !pImpl->mConnectionString.empty();
}

/// Security level
void Details::setSecurityLevel(UAuth::SecurityLevel securityLevel) noexcept
{
    pImpl->mSecurityLevel = securityLevel;
}

UAuth::SecurityLevel Details::getSecurityLevel() const noexcept
{
    return pImpl->mSecurityLevel;
}
