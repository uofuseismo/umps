#include <iostream>
#include <string>
#include <array>
#include <set>
#include <thread>
#include <mutex>
#include <sqlite3.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/authentication/service.hpp"
#include "umps/messaging/authentication/authenticator.hpp"
#include "umps/messaging/authentication/sqlite3Authenticator.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

using namespace UMPS::Messaging::Authentication;

#define ZAP_OKAY "OK"
#define ZAP_SUCCESS "200"
#define ZAP_CLIENT_ERROR "400"
#define ZAP_SERVER_ERROR "500"

/// Magic place where ZMQ will send authentication requests to.
#define ZAP_ENDPOINT  "inproc://zeromq.zap.01"

class Service::ServiceImpl
{
public:
    /// C'tor 
    ServiceImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPipe(std::make_unique<zmq::socket_t> (*mContext,
                                               zmq::socket_type::pair)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mAuthenticator(std::make_shared<SQLite3Authenticator> (mLogger))
    {
        makeEndPointName();
    }
    explicit ServiceImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPipe(std::make_unique<zmq::socket_t> (*mContext,
                                               zmq::socket_type::pair)),
        mLogger(logger)
    {   
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        mAuthenticator
            = std::make_shared<SQLite3Authenticator> (mLogger);
        makeEndPointName();
    }
    explicit ServiceImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mPipe(std::make_unique<zmq::socket_t> (*mContext,
                                               zmq::socket_type::pair)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating context");
            mContext = std::make_shared<zmq::context_t> (1);
            mPipe = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::pair);
        }
        mAuthenticator
            = std::make_shared<SQLite3Authenticator> (mLogger);
        makeEndPointName();
    }
    ServiceImpl(std::shared_ptr<zmq::context_t> &context,
                            std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(context),
        mPipe(std::make_unique<zmq::socket_t> (*mContext,
                                               zmq::socket_type::pair)),
        mLogger(logger)
    {   
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating context");
            mContext = std::make_shared<zmq::context_t> (1);
            mPipe = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::pair);
        }
        mAuthenticator = std::make_shared<SQLite3Authenticator> (mLogger);
        makeEndPointName();
    }
    /// Convenience function to make endpoint name
    void makeEndPointName()
    {
        std::ostringstream address;
        address << static_cast<void const *> (this);
        mEndPoint = "inproc://" + address.str() + ".inproc";
        //std::cout <<  mEndPoint << std::endl;
        mPipe->set(zmq::sockopt::linger, 1); 
        mPipe->bind(mEndPoint);
    }
    /// Determines if the service was started
    bool isRunning() const noexcept
    {
        std::scoped_lock lock(mMutex);
        auto running = mRunning;
        return running;
    }
    /// Start the service
    void start()
    {
        std::scoped_lock lock(mMutex);
        mRunning = true;
    }
    /// Stop the service
    void stop()
    {
        std::scoped_lock lock(mMutex);
        mRunning = false;
    }
//private:
    mutable std::mutex mMutex;
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mPipe;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::shared_ptr<IAuthenticator> mAuthenticator;
    //std::thread mThread;
    std::string mEndPoint;
    std::chrono::milliseconds mPollTimeOutMS{-1};
    //bool mHaveThread = false;
    bool mHavePipe = false;
    bool mRunning = false;
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> ())
{
}

Service::Service(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (logger))
{
}

Service::Service(
    std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<ServiceImpl> (context))
{
}

Service::Service(
    std::shared_ptr<zmq::context_t> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (context, logger))
{
}

/// Destructor
Service::~Service()
{
   stop();
}
// = default;

/// Start the thread authenticator
void Service::start()
{
    if (isRunning())
    {
        pImpl->mLogger->warn("Service is running.  Attemping to stop.");
        stop();
    }
    zmq::socket_t pipe(*pImpl->mContext, zmq::socket_type::pair);
    pipe.set(zmq::sockopt::linger, 1);
    try
    {
        pipe.connect(pImpl->mEndPoint);
    }
    catch (const std::exception &e)
    {
        auto error = "Failed to connect pipe.  ZMQ failed with "
                  + std::string(e.what());
        throw std::runtime_error(e.what());
    }
    // Create a ZAP socket
    pImpl->mLogger->debug("Binding to ZAP socket...");
    zmq::context_t zapContext(0);
    zmq::socket_t zap(*pImpl->mContext, zmq::socket_type::rep);
    zap.bind(ZAP_ENDPOINT);

    // Let pipe know I'm ready
    pipe.send(zmq::message_t{}, zmq::send_flags::none);
    pImpl->mLogger->debug("Starting authenticator on endpoint "
                        + pImpl->mEndPoint);
    //Authenticator authenticator(pImpl->mContext, pImpl->mLogger);
    //pImpl->mAuthenticator->start(); 
    //auto zap = pImpl->mAuthenticator->getZapSocket(); 
    const int nPollItems = 2;
    pipe.send(zmq::message_t{}, zmq::send_flags::none); // Signal I'm ready
    zmq::pollitem_t items[] =
    {
        {pipe.handle(), 0, ZMQ_POLLIN, 0},
        {zap.handle(),  0, ZMQ_POLLIN, 0}
    };
    pImpl->start();
    bool keepRunning = true;
    while (keepRunning)
    {
        keepRunning = isRunning();
        pImpl->mLogger->debug("Waiting for message...");
        zmq::poll(items, nPollItems, pImpl->mPollTimeOutMS);
        //pImpl->mLogger->debug("Message received!");
        // Handle API requests.  This is how the authenticator is configured
        // by the program's main thread.
        if (items[0].revents & ZMQ_POLLIN)
        {
            pImpl->mLogger->debug("API request received");
            std::vector<zmq::message_t> messagesReceived;
            zmq::recv_result_t receivedResult =
                zmq::recv_multipart(pipe,
                                    std::back_inserter(messagesReceived),
                                    zmq::recv_flags::none);
            if (*receivedResult > 0)
            {
                auto nAddresses = messagesReceived.size() - 1;
                std::string command{messagesReceived[0].to_string()};
                if (command == "DENY")
                {
                    for (int i = 0; i < static_cast<int> (nAddresses); ++i)
                    {
                        auto address = messagesReceived.at(i + 1).to_string();
                        if (pImpl->mAuthenticator->isWhitelisted(address))
                        {
                            //pImpl->mAuthenticator->removeFromWhitelist(address);
                        }
                        //pImpl->mAuthenticator->addToBlacklist(address); 
                    }
                }
                else if (command == "ALLOW")
                {
                    for (int i = 0; i < static_cast<int> (nAddresses); ++i)
                    {
                        auto address = messagesReceived.at(i + 1).to_string();
			if (pImpl->mAuthenticator->isBlacklisted(address))
                        {
                            //pImpl->mAuthenticator->removeFromBlacklist(address);
                        }
                        //pImpl->mAuthenticator->addToWhitelist(address);
                    }
                }
                else if (command == "CURVE")
                {
                    auto domain = messagesReceived.at(1).to_string();
                    auto location = messagesReceived.at(2).to_string();
                }
                else if (command == "PLAIN")
                {
                    auto domain = messagesReceived.at(1).to_string();
                    auto password = messagesReceived.at(2).to_string();
                }
                else if (command == "TERMINATE")
                {
                    pImpl->mLogger->debug("Terminating service");
                    pImpl->stop(); // Stop the service
                    keepRunning = false;
                }
                else
                {
                    pImpl->mLogger->error("Unhandled command: " + command);
                }
            }
        } // End API request
        // Handle ZAP requests.  These come to the poller from
        // inproc://zeromq.zap.01.
        if (items[1].revents & ZMQ_POLLIN)
        {
            pImpl->mLogger->debug("ZAP request received");
            zmq::multipart_t messageReceived(zap);
/*
            std::vector<zmq::message_t> messagesReceived;
            zmq::recv_result_t receivedResult =
                zmq::recv_multipart(zap,
                                    std::back_inserter(messagesReceived),
                                    zmq::recv_flags::none);
*/
#ifndef NDEBUG
            assert(messageReceived.size() >= 6);
#endif
            // Order is defined in:
            // https://rfc.zeromq.org/spec/27/
            auto domain    = messageReceived.at(2).to_string(); // e.g., global
            auto ipAddress = messageReceived.at(3).to_string();
            auto identity  = messageReceived.at(4).to_string(); // Originating socket ID
            auto mechanism = messageReceived.at(5).to_string();
            std::string statusCode = ZAP_CLIENT_ERROR; 
            std::string statusText;
            // Is the IP address blacklisted?
            if (pImpl->mAuthenticator->isBlacklisted(ipAddress))
            {
                statusCode = ZAP_CLIENT_ERROR;
                statusText = "Address blacklisted";
            } 
            else
            {
                // Handle different mechanisms.
                if (mechanism == "NULL")
                {
                    statusCode = ZAP_SUCCESS;
                    statusText = ZAP_OKAY;
                }
                else if (mechanism == "PLAIN")
                {
                    //pImpl->mAuthenticator.verifyPlain( );
                    auto user = messageReceived[5].to_string();
                    auto password = messageReceived[6].to_string();
                    Certificate::UserNameAndPassword plainText;
                    bool verified = false;
                    try
                    {
                        plainText.setUserName(user);
                        plainText.setPassword(password);
                    }
                    catch (const std::exception &e)
                    {
                        pImpl->mLogger->error("Failed to set username/pwd");
                    }

verified = true;
                    if (verified)
                    {
                        statusCode = ZAP_SUCCESS;
                        statusText = ZAP_OKAY;
                    }
                    else
                    {
                        statusCode = ZAP_CLIENT_ERROR;
                        statusText = "Username/password failed";  
                    }
                }
                else if (mechanism == "CURVE")
                {
                    bool verified = true;
                    if (messageReceived.at(6).size() != 32)
                    {
                        statusCode = ZAP_CLIENT_ERROR;
                        statusText = "Key is invalid length";
                    }
                    else
                    {
                        auto keyPtr = reinterpret_cast<const uint8_t *>
                                      (messageReceived.at(6).data());
                        std::array<uint8_t, 32> publicKey;
                        std::copy(keyPtr, keyPtr + 32, publicKey.data());
                        Certificate::Keys key;
                        try
                        {
                            key.setPublicKey(publicKey);
                        }
                        catch (const std::exception &e)
                        {
                            pImpl->mLogger->error("Failed to set public key");
                            statusCode = ZAP_SERVER_ERROR;
                            statusText = "Failed to set public key";
                        }
verified = true;
                        if (key.havePublicKey())
                        {
                        }                        
                        if (verified)
                        {
                            statusCode = ZAP_SUCCESS;
                            statusText = ZAP_OKAY;
                        }
                        else
                        {
                            statusCode = ZAP_CLIENT_ERROR;
                            statusText = "Key could not be verified";
                        } 
                    }
                }
                else
                {
                    statusCode = ZAP_CLIENT_ERROR;
                    statusText = "Security mechanism: " + mechanism
                                + " not supported"; 
                }
            }
            if (true)
            {
                for (auto &m : messageReceived)
                {
                    std::cout << m << std::endl;
                }
            }
            // Format result.  The order is defined in:
            // https://rfc.zeromq.org/spec/27/
            zmq::multipart_t reply;
            reply.addstr(messageReceived[0].to_string()); // Version
            reply.addstr(messageReceived[1].to_string()); // Sequence Number 
            reply.addstr(statusCode);
            reply.addstr(statusText);
            reply.addstr("Joe");
            reply.addstr(""); // Always end with this
            reply.send(zap);
 
        }
    } // Loop on running
    pImpl->mLogger->debug("Exiting threadAuthenticator polling loop");
    pipe.close();
}

/// Tell thread running service to stop
void Service::stop()
{
    if (isRunning())
    {
        pImpl->mLogger->debug("Sending TERMINATE message");
        pImpl->mPipe->send(zmq::str_buffer("TERMINATE"),
                           zmq::send_flags::none);
    }
//    pImpl->stop(); // Tell thread running service to stop
/*
    if (pImpl->mHaveThread)
    {
        pImpl->mLogger->debug("Terminating threadAuthenticator...");
        //pImpl->mThread.join();
        //pImpl->mHaveThread = false;
    }
    if (pImpl->mHavePipe)
    {
        pImpl->mLogger->debug("Closing pipe...");
    }
*/
}

/// Is the service running?
bool Service::isRunning() const noexcept
{
    return pImpl->isRunning();
}

/// Whitelist an address
void Service::whitelist(const std::string &address)
{
    if (address.empty())
    {
        throw std::invalid_argument("Address is empty");
    }
    if (!isRunning()){throw std::runtime_error("Service not started");}
    const std::string commandType = "ALLOW";
    zmq::const_buffer header{commandType.data(), commandType.size()};
    pImpl->mPipe->send(header, zmq::send_flags::sndmore);
    zmq::const_buffer buffer{address.data(), address.size()};
    pImpl->mPipe->send(buffer);

}

/// Blacklist an address
void Service::blacklist(const std::string &address)
{
    if (address.empty())
    {
        throw std::invalid_argument("Address is empty");
    }
    if (!isRunning()){throw std::runtime_error("Service not started");}
    const std::string commandType = "DENY";
    zmq::const_buffer header{commandType.data(), commandType.size()};
    pImpl->mPipe->send(header, zmq::send_flags::sndmore);
    zmq::const_buffer buffer{address.data(), address.size()};
    pImpl->mPipe->send(buffer);
}

/// Configures a curve server 
