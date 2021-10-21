#include "umps/messaging/authentication/grasslands.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"

using namespace UMPS::Messaging::Authentication;

class Grasslands::GrasslandsImpl
{
public:
    GrasslandsImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {    
    }    
    explicit GrasslandsImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
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
        pImpl->mLogger->info("Address: " + address + " is not blacklisted");
    }
    return std::pair(okayStatus(), okayMessage());
}   

std::pair<std::string, std::string> Grasslands::isWhitelisted(
        const std::string &address) const noexcept
{
    if (pImpl->mLogger->getLevel() >= UMPS::Logging::Level::INFO)
    {
        pImpl->mLogger->info("Address: " + address + " is whitelisted");
    }
    return std::pair(okayStatus(), okayMessage());
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
            pImpl->mLogger->info("User is allowed");
        }
    }
    return std::pair(okayStatus(), okayMessage());
}

std::pair<std::string, std::string> Grasslands::isValid(
    const Certificate::Keys &) const noexcept
{
    if (pImpl->mLogger->getLevel() >= UMPS::Logging::Level::INFO)
    {
        pImpl->mLogger->info("User public key is allowed");
    }
    return std::pair(okayStatus(), okayMessage());
}   
