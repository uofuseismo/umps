#include <iostream>
#include <string>
#include <array>
#include <set>
#include <thread>
#include <mutex>
#include <sqlite3.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/authentication/service.hpp"
#include "umps/authentication/authenticator.hpp"
#include "umps/authentication/grasslands.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

using namespace UMPS::Authentication;

/*
#define ZAP_OKAY "OK"
#define ZAP_SUCCESS "200"
#define ZAP_CLIENT_ERROR "400"
#define ZAP_SERVER_ERROR "500"
*/

/// Magic place where ZMQ will send authentication requests to.
#define ZAP_ENDPOINT  "inproc://zeromq.zap.01"

class Service::ServiceImpl
{
public:
    /*
    /// C'tor 
    ServiceImpl(std::shared_ptr<zmq::context_t> context,
                std::shared_ptr<UMPS::Logging::ILog> logger,
                std::shared_ptr<IAuthenticator> authenticator,
                int) :
        mContext(context),
        mLogger(logger),
        mAuthenticator(authenticator),
        mDeprecatedContext(true)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
        }
        if (authenticator == nullptr)
        {
            mAuthenticator = std::make_shared<Grasslands> (mLogger);
        }
        mPipe = std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::pair);
        makeEndPointName();
    }
    */
    ServiceImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                std::shared_ptr<UMPS::Logging::ILog> logger,
                std::shared_ptr<IAuthenticator> authenticator) :
        mContext(context),
        mLogger(logger),
        mAuthenticator(authenticator)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> (1);
        }
        if (authenticator == nullptr)
        {
            mAuthenticator = std::make_shared<Grasslands> (mLogger);
        }
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mPipe = std::make_unique<zmq::socket_t> (*contextPtr,
                                                 zmq::socket_type::pair);
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
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::unique_ptr<zmq::socket_t> mPipe{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::shared_ptr<IAuthenticator> mAuthenticator{nullptr};
    //std::thread mThread;
    std::string mEndPoint;
    std::chrono::milliseconds mPollTimeOutMS{-1};
    //bool mHaveThread{false};
    bool mHavePipe{false};
    bool mRunning{false};
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> (nullptr, nullptr, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger, nullptr))
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<ServiceImpl> (context, nullptr, nullptr))   
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (context, logger, nullptr))   
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
                 std::shared_ptr<IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (nullptr, logger, authenticator))
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (context, nullptr, authenticator))   
{
}

Service::Service(std::shared_ptr<UMPS::Messaging::Context> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger,
                 std::shared_ptr<IAuthenticator> &authenticator) :
    pImpl(std::make_unique<ServiceImpl> (context, logger, authenticator)) 
{
}

/// Destructor
Service::~Service()
{
   stop();
}

/// Start the thread authenticator
void Service::start()
{
    if (isRunning())
    {
        pImpl->mLogger->warn("Service is running.  Attemping to stop.");
        stop();
    }
    zmq::context_t *contextPtr{nullptr};
    contextPtr
        = reinterpret_cast<zmq::context_t *> (pImpl->mContext->getContext());
    zmq::socket_t pipe(*contextPtr, zmq::socket_type::pair);
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
    zmq::socket_t zap(*contextPtr, zmq::socket_type::rep);
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
                std::string command{messagesReceived[0].to_string()};
/*
                auto nAddresses = messagesReceived.size() - 1;
                if (command == "DENY")
                {
                    for (int i = 0; i < static_cast<int> (nAddresses); ++i)
                    {
                        auto address = messagesReceived.at(i + 1).to_string();
                        if (pImpl->mAuthenticator->isWhitelisted(address) !=
                            ValidationResult::BLACKLISTED)
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
			if (pImpl->mAuthenticator->isBlacklisted(address) ==
                            ValidationResult::BLACKLISTED)
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
*/
                if (command == "TERMINATE")
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
#ifndef NDEBUG
            assert(messageReceived.size() >= 6);
#endif
            // Order is defined in:
            // https://rfc.zeromq.org/spec/27/
            auto domain    = messageReceived.at(2).to_string(); // e.g., global
            auto ipAddress = messageReceived.at(3).to_string();
            auto identity  = messageReceived.at(4).to_string(); // Originating socket ID
            auto mechanism = messageReceived.at(5).to_string();
            // Default failure
            std::string statusCode = IAuthenticator::serverErrorStatus();
            std::string statusText = "Unhandled request";
            // Is the IP address blacklisted?
            std::tie (statusCode, statusText)
                = pImpl->mAuthenticator->isBlacklisted(ipAddress);
            if (statusCode == IAuthenticator::okayStatus())
            {
                // Handle different mechanisms.
                if (mechanism == "NULL") // Check nothing - blacklist enough
                {
                    statusCode = IAuthenticator::okayStatus();
                    statusText = IAuthenticator::okayMessage();
                }
                else if (mechanism == "PLAIN") // Check username/passsword
                {
                    statusCode = IAuthenticator::serverErrorStatus();
                    statusText = "Unhandled PLAIN logic";
                    //pImpl->mAuthenticator.verifyPlain( );
                    auto user = messageReceived[5].to_string();
                    auto password = messageReceived[6].to_string();
                    Certificate::UserNameAndPassword plainText;
                    try
                    {
                        plainText.setUserName(user);
                        plainText.setPassword(password);
                    }
                    catch (const std::exception &e)
                    {
                        pImpl->mLogger->error("Failed to set username/pwd");
                    }
                    // Check the username and password
                    std::tie(statusCode, statusText) 
                        = pImpl->mAuthenticator->isValid(plainText);
                }
                else if (mechanism == "CURVE") // Check public key 
                {
                    statusCode = IAuthenticator::serverErrorStatus();
                    statusText = "Unhandled CURVE logic"; 
                    if (messageReceived.at(6).size() != 32)
                    {
                        statusCode = IAuthenticator::clientErrorStatus();
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
                            statusCode = IAuthenticator::serverErrorStatus();
                            statusText = "Failed to set public key";
                        }
                        if (key.havePublicKey())
                        {
                            std::tie(statusCode, statusText)
                                = pImpl->mAuthenticator->isValid(key);
                        }
                    }
                }
                else // Check on mechanism
                {
                    statusCode = IAuthenticator::clientErrorStatus();
                    statusText = "Security mechanism: " + mechanism
                                + " not supported"; 
                } // End check on mechanism
            } // End check on not blacklisted
            /*
            if (true)
            {
                for (auto &m : messageReceived)
                {
                    std::cout << m << std::endl;
                }
            }
            */
            if (statusCode == IAuthenticator::okayStatus())
            {
                pImpl->mLogger->info("Allowing " + mechanism
                                   + " connection from: " + ipAddress);
            }
            else
            {
                pImpl->mLogger->debug("Blocking connection from: " + ipAddress);
            } 
            // Format result.  The order is defined in:
            // https://rfc.zeromq.org/spec/27/
            zmq::multipart_t reply;
            reply.addstr(messageReceived[0].to_string()); // Version
            reply.addstr(messageReceived[1].to_string()); // Sequence Number 
            reply.addstr(statusCode);
            reply.addstr(statusText);
            reply.addstr("UAuth");
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

/// Minimum permissions
UserPrivileges Service::getMinimumUserPrivileges() const
{
    return pImpl->mAuthenticator->getMinimumUserPrivileges();
}
