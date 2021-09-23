#include <mutex>
#include <string>
#include <set>
#include "umps/messaging/authentication/authenticator.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

using namespace UMPS::Messaging::Authentication;

class AuthenticationImpl
{
public:
    AuthenticationImpl() :
        mLogger(std::make_shared<URTS::Logging::StdOut> ())
    {
    }
    /// Add to blacklist
    void addToBlacklist(const std::string &address)
    {
        std::scoped_lock lock(mMutex);
        if (!mBlacklist.contains(address))
        {
            if (mWhitelist.contains(address))
            {
                auto errmsg = "Remove " + address
                            + " from whitelist before blacklisting";
                mLogger.error(errmsg);
                throw std::invalid_argument(errmsg);
            }
            mLogger.debug("Adding: " + address + " to blacklist");
            mBlacklist.insert(address);
        }
        else
        {
            mLogger.debug("Address: " + address + " already on blacklist");
        }
    }
    /// Add to whitelist
    void addToWhitelist(const std::string &address)
    {
        std::scoped_lock lock(mMutex);
        if (!mWhitelist.contains(address))
        {
            if (mBlacklist.contains(address))
            {
                auto errmsg = "Remove " + address
                            + " from blacklist before whitelisting";
                mLogger.error(errmsg);
                throw std::invalid_argument(errmsg);
            }
            mLogger.debug("Adding: " + address + " to whitelist");
            mWhitelist.insert(address);
        }
        else
        {
            mLogger.debug("Address: " + address + " already on whitelist");
        }
        
    }
    /// Blacklisted?
    bool isBlacklisted(const std::string &address) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mBlacklist.contains(address);
    }
    /// Whitelisted?
    bool isWhitelisted(const std::string &address) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mWhitelist.contains(address);
    }
    mutable std::mutex mMutex;
    std::shared_ptr<URTS::Logging::ILog> mLogger;
    std::set<std::string> mBlacklist;
    std::set<std::string> mWhiteList;
};

/// C'tor
Authentication::Authentication() :
    pImpl(std::make_unique<AuthenticationImpl> ())
{
}

/// Destructor
Authentication::~Authentication() = default;

/// Black list
void Authentication::addToBlacklist(const std::string &address)
{
    pImpl->addToBlacklist(address); 
}

bool Authentication::isBlacklisted(address) const noexcept
{
    return pImpl->isBlacklisted(address);
} 

/// White list
void Authentication::addToWhitelist(const std::string &address)
{
    pImpl->addToWhitelist(address);
}

bool Authentication::isWhitelisted(address) const noexcept
{
    return pImpl->isWhitelisted(address);
}
