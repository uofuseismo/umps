#include <iostream>
#include <mutex>
#include <string>
#include <set>
#include <zmq.hpp>
#include "umps/messaging/authentication/authenticator.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

/// Magic place where ZMQ will send authentication requests to.
#define ZAP_ENDPOINT  "inproc://zeromq.zap.01"

using namespace UMPS::Messaging::Authentication;

class Authenticator::AuthenticatorImpl
{
public:
    AuthenticatorImpl() :
/*
        mContext(std::make_shared<zmq::context_t> (1)),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)),
*/
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    explicit AuthenticatorImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
/*
        mContext(std::make_shared<zmq::context_t> (1)),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)),
*/
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
/*
    explicit AuthenticatorImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)), 
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()) 
    {
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating context");
            mContext = std::make_shared<zmq::context_t> (1);
            mZapSocket = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::rep);
        }
    }
    AuthenticatorImpl(std::shared_ptr<zmq::context_t> &context,
                      std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(context),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)), 
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating cotnext");
            mContext = std::make_shared<zmq::context_t> (1);
            mZapSocket = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::rep);
        }
    }
*/
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
                mLogger->error(errmsg);
                throw std::invalid_argument(errmsg);
            }
            mLogger->debug("Adding: " + address + " to blacklist");
            mBlacklist.insert(address);
        }
        else
        {
            mLogger->debug("Address: " + address + " already on blacklist");
        }
    }
    /// Remove from blacklist
    void removeFromBlacklist(const std::string &address) noexcept
    {
        std::scoped_lock lock(mMutex);
        if (mBlacklist.contains(address))
        {
            mBlacklist.erase(address);
            mLogger->debug("Removing: " + address + " from blacklist");
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
                mLogger->error(errmsg);
                throw std::invalid_argument(errmsg);
            }
            mLogger->debug("Adding: " + address + " to whitelist");
            mWhitelist.insert(address);
        }
        else
        {
            mLogger->debug("Address: " + address + " already on whitelist");
        }
        
    }
    /// Remove from whitelist
    void removeFromWhitelist(const std::string &address) noexcept
    {
        std::scoped_lock lock(mMutex);
        if (mWhitelist.contains(address))
        {
            mWhitelist.erase(address);
            mLogger->debug("Removing: " + address + " from whitelist");
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
///private:
    mutable std::mutex mMutex;
/*
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mZapSocket;
*/
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::set<std::string> mBlacklist;
    std::set<std::string> mWhitelist;
    std::set<std::pair<std::string, std::string>> mPasswords;
    bool mHaveZapSocket = false;
};

/// C'tors
Authenticator::Authenticator() :
    pImpl(std::make_unique<AuthenticatorImpl> ())
{
}

Authenticator::Authenticator(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<AuthenticatorImpl> (logger))
{
}

/*
Authenticator::Authenticator(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<AuthenticatorImpl> (context))
{
}

Authenticator::Authenticator(std::shared_ptr<zmq::context_t> &context,
                             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<AuthenticatorImpl> (context, logger))
{
}
*/

/// Destructor
Authenticator::~Authenticator() = default;

/// Black list
void Authenticator::addToBlacklist(const std::string &address)
{
    pImpl->addToBlacklist(address); 
}

bool Authenticator::isBlacklisted(const std::string &address) const noexcept
{
    return pImpl->isBlacklisted(address);
} 

void Authenticator::removeFromBlacklist(const std::string &address) noexcept
{
    return pImpl->removeFromBlacklist(address);
}

/// White list
void Authenticator::addToWhitelist(const std::string &address)
{
    pImpl->addToWhitelist(address);
}

bool Authenticator::isWhitelisted(const std::string &address) const noexcept
{
    return pImpl->isWhitelisted(address);
}

void Authenticator::removeFromWhitelist(const std::string &address) noexcept
{
    return pImpl->removeFromWhitelist(address);
}

/// Start the authenticator
/*
void Authenticator::start()
{
    pImpl->mLogger->debug("Starting authenticator ZAP socket...");
    stop();
    try
    {
        pImpl->mZapSocket->set(zmq::sockopt::linger, 1);
    }
    catch (const std::exception &e)
    {
        auto error = "Failed to set socket option.  ZMQ failed with: "
                   + std::string(e.what());
        throw std::runtime_error(error);
    }
    try
    {
        pImpl->mZapSocket->connect(ZAP_ENDPOINT);
    }
    catch (const std::exception &e)
    {
        auto error = "Failed to bind to ZAP.  ZMQ failed with: " 
                   + std::string(e.what());
        throw std::runtime_error(error);
    }
    pImpl->mHaveZapSocket = true;
}
*/

void Authenticator::stop()
{
    if (pImpl->mHaveZapSocket)
    {
        pImpl->mLogger->debug("Stopping authenticator ZAP socket...");
//        pImpl->mZapSocket->close();
        pImpl->mHaveZapSocket = false;
    }
}

/// Get a handle on the zap socket
/*
zmq::socket_t* Authenticator::getZapSocket()
{
    return &*pImpl->mZapSocket;
}
*/
