#include "umps/authentication/grasslands.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "umps/logging/standardOut.hpp"

using namespace UMPS::Authentication;

class Grasslands::GrasslandsImpl
{
public:
    GrasslandsImpl() :
        mLogger(std::make_shared<UMPS::Logging::StandardOut> ())
    {    
    }    
    explicit GrasslandsImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StandardOut> ();
        }
    }
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    const UserPrivileges mPrivileges{UserPrivileges::ReadOnly};
};

/// C'tor
Grasslands::Grasslands() :
    pImpl(std::make_unique<GrasslandsImpl> ())
{
}

/// C'tor
Grasslands::Grasslands(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<GrasslandsImpl> (logger))
{
}

/// Destructor
Grasslands::~Grasslands() = default;
    
std::pair<std::string, std::string> Grasslands::isBlacklisted(
        const std::string &address) const noexcept
{
    if (pImpl->mLogger->getLevel() >= UMPS::Logging::Level::INFO)
    {
        pImpl->mLogger->info("Grasslands address: "
                           + address + " is not blacklisted");
    }
    return std::pair{okayStatus(), okayMessage()};
}   

std::pair<std::string, std::string> Grasslands::isWhitelisted(
        const std::string &address) const noexcept
{
    if (pImpl->mLogger->getLevel() >= UMPS::Logging::Level::INFO)
    {
        pImpl->mLogger->info("Grasslands address: "
                           + address + " is whitelisted");
    }
    return std::pair{okayStatus(), okayMessage()};
}

std::pair<std::string, std::string> Grasslands::isValid(
    const Certificate::UserNameAndPassword &userNameAndPassword) const noexcept
{
    if (pImpl->mLogger->getLevel() >= UMPS::Logging::Level::INFO)
    {
        if (userNameAndPassword.haveUserName())
        {
            auto name = userNameAndPassword.getUserName();
            pImpl->mLogger->info("User: " + name + " is allowed");
        }
        else
        {
            pImpl->mLogger->info("Grasslands user is allowed");
        }
    }
    return std::pair{okayStatus(), okayMessage()};
}

std::pair<std::string, std::string> Grasslands::isValid(
    const Certificate::Keys &) const noexcept
{
    if (pImpl->mLogger->getLevel() >= UMPS::Logging::Level::INFO)
    {
        pImpl->mLogger->info("Grasslands user public key is allowed");
    }
    return std::pair{okayStatus(), okayMessage()};
}   

UserPrivileges Grasslands::getMinimumUserPrivileges() const noexcept
{
    return pImpl->mPrivileges;
}
