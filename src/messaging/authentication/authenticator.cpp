#include <iostream>
#include <mutex>
#include <string>
#include <set>
#include <zmq.hpp>
#include "umps/messaging/authentication/authenticator.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

using namespace UMPS::Messaging::Authentication;

class Authenticator::AuthenticatorImpl
{
public:
    void makeEndPointName()
    {
        std::ostringstream address;
        address << static_cast<void const *> (this);
        mEndPoint = "inproc://" + address.str() + ".inproc";
        //std::cout <<  mEndPoint << std::endl;
    }
    AuthenticatorImpl() :
        mContext(std::make_shared<zmq::context_t> (0)),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
        makeEndPointName();
    }
    explicit AuthenticatorImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (0)),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        makeEndPointName();
    }
    explicit AuthenticatorImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)), 
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()) 
    {
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating context");
            mContext = std::make_shared<zmq::context_t> (0);
            mZapSocket = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::rep);
        }
        makeEndPointName();
    }
    AuthenticatorImpl(std::shared_ptr<zmq::context_t> &context,
                      std::shared_ptr<UMPS::Logging::ILog> &logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating cotnext");
            mContext = std::make_shared<zmq::context_t> (0);
            mZapSocket = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::rep);
        }
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
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mZapSocket;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::set<std::string> mBlacklist;
    std::set<std::string> mWhitelist;
    std::set<std::pair<std::string, std::string>> mPasswords;
    std::string mEndPoint;
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

Authenticator::Authenticator(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<AuthenticatorImpl> (context))
{
}

Authenticator::Authenticator(std::shared_ptr<zmq::context_t> &context,
                             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<AuthenticatorImpl> (context, logger))
{
}

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

/// White list
void Authenticator::addToWhitelist(const std::string &address)
{
    pImpl->addToWhitelist(address);
}

bool Authenticator::isWhitelisted(const std::string &address) const noexcept
{
    return pImpl->isWhitelisted(address);
}

/// Start the authenticator
void Authenticator::start()
{
    pImpl->mLogger->debug("Starting authenticator ZAP socket...");
    stop();
    pImpl->mZapSocket->set(zmq::sockopt::linger, 1);
    pImpl->mZapSocket->bind("inproc://zeromq.zap.01"); // ZMQ magic happens
    pImpl->mHaveZapSocket = true;
}

void Authenticator::stop()
{
    if (pImpl->mHaveZapSocket)
    {
        pImpl->mLogger->debug("Stopping authenticator ZAP socket...");
        pImpl->mZapSocket->close();
        pImpl->mHaveZapSocket = false;
    }
}
