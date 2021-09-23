#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messageFormats/message.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

using namespace UMPS::Messaging::PublisherSubscriber;

/*
class Publisher::PublisherImpl
{
public:
    /// C'tor
    PublisherImpl() :
        //mContext(std::make_unique<zmq::context_t> (1)),
        //mPublisher(std::make_unique<zmq::socket_t> (*mContext,
        //                                            zmq::socket_type::pub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
        mContext = std::make_shared<void *> (zmq_ctx_new());
        mPublisher = zmq_socket(*mContext, ZMQ_PUB);
    }
    /// C'tor
    PublisherImpl(std::shared_ptr<void *> &context) :
        mContext(context),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mMadeContext(false)
    {
        mMadeContext = false;
        if (mContext == nullptr)
        {
            mLogger->error("Context is NULL - creating one");
            mContext = std::make_shared<void *> (zmq_ctx_new());
            mMadeContext = true;
        }
        mPublisher = zmq_socket(*mContext, ZMQ_PUB);
    }
    /// C'tor
    PublisherImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
//        mContext(std::make_unique<zmq::context_t> (0)),
//        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
//                                                    zmq::socket_type::pub)),
        mLogger(logger)
    {
        mContext = std::make_shared<void *> (zmq_ctx_new());
        mPublisher = zmq_socket(*mContext, ZMQ_PUB);
    }
    /// Destructor
    ~PublisherImpl()
    {
        if (mPublisher){zmq_close(mPublisher);}
        if (mContext && mMadeContext){zmq_ctx_destroy(*mContext);}
        mEndPoints.clear();
        mPublisher = nullptr;
        mContext = nullptr;
        mLogger = nullptr;
        mMadeContext = true;
    }
    /// The context is the container for all sockets in a single
    /// process.  Multiple contexts would behave like separate
    /// zeromq instances.
    const int nContexts = 1;
    //zmq::context_t mContext{nContexts};
    //zmq::socket_t mPublisher{mContext, zmq::socket_type::pub};
    void *mPublisher = nullptr;
    std::shared_ptr<void *> mContext = nullptr;
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    bool mMadeContext = true;
};
*/

class Publisher::PublisherImpl
{
public:
    /// C'tor
    PublisherImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    /// C'tor
    explicit PublisherImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    /// C'tor
    explicit PublisherImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(std::make_shared<zmq::context_t> (1)),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// C'tor
    PublisherImpl(std::shared_ptr<zmq::context_t> &context,
                  std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(context),
        mPublisher(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::pub)),
        mLogger(logger)
    {
    }
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mPublisher;
    std::map<std::string, bool> mEndPoints;
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
};

/// C'tor
Publisher::Publisher() :
    pImpl(std::make_unique<PublisherImpl> ())
{
}

/// C'tor
Publisher::Publisher(std::shared_ptr<zmq::context_t> &context) :
   pImpl(std::make_unique<PublisherImpl> (context))
{
}

/// C'tor with logger
Publisher::Publisher(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<PublisherImpl> (logger))
{
}

/// Move c'tor
Publisher::Publisher(Publisher &&publisher) noexcept
{
    *this = std::move(publisher);
}

/// Destructor
Publisher::~Publisher() = default;

/// Move assignment
Publisher& Publisher::operator=(Publisher &&publisher) noexcept
{
    if (&publisher == this){return *this;}
    pImpl = std::move(publisher.pImpl);
    return *this;
}

/// Bind to an address if not already done so
void Publisher::bind(const std::string &endPoint)
{
    auto idx = pImpl->mEndPoints.find(endPoint);
    if (idx == pImpl->mEndPoints.end())
    {
        pImpl->mLogger->debug("Attempting to bind to: " + endPoint);
    }
    else
    {
        if (idx->second)
        {
            pImpl->mLogger->info("Already bound to end point: " + endPoint);
            return;
        }
        else
        {
            pImpl->mLogger->info("Attempting to rebind to: " + endPoint);
        }
    }
    try
    {
        pImpl->mPublisher->bind(endPoint);
    }
    catch (const std::exception &e)
    {
        auto errorMsg = "Failed to connect to endpoint: " + endPoint
                      + "\nZeroMQ failed with: " + std::string(e.what());
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
}

/// Send a message
void Publisher::send(const MessageFormats::IMessage &message)
{
    auto messageType = message.getMessageType();
    if (messageType.empty())
    {
        pImpl->mLogger->debug("Message type is empty");
    }
    auto cborMessage = std::string(message.toCBOR());
    if (cborMessage.empty())
    {
        pImpl->mLogger->debug("CBOR message is empty");
    }
    //pImpl->mLogger->debug("Sending message of type: " + messageType);
    zmq::const_buffer header{messageType.data(), messageType.size()};
    pImpl->mPublisher->send(header, zmq::send_flags::sndmore);
    zmq::const_buffer buffer{cborMessage.data(), cborMessage.size()};
    pImpl->mPublisher->send(buffer);
}

/*
/// Bind to an address if not already done so
void Publisher::bind(const std::string &endPoint)
{
    auto idx = pImpl->mEndPoints.find(endPoint);
    if (idx == pImpl->mEndPoints.end())
    {
        pImpl->mLogger->debug("Attempting to bind to: " + endPoint);
    }
    else
    {
        if (idx->second)
        {
            pImpl->mLogger->info("Already bound to end point: " + endPoint);
            return;
        }
        else
        {
            pImpl->mLogger->info("Attempting to rebind to: " + endPoint);
        }
    }
    // Finally attempt to bind to address
    int highWaterMark = 1000;
    auto e = zmq_setsockopt(pImpl->mPublisher, ZMQ_SNDHWM,
                   &highWaterMark, sizeof(highWaterMark));
    auto zmqError = zmq_bind(pImpl->mPublisher, endPoint.c_str());
    if (zmqError != 0)
    {
        auto zmqErrorMsg = zmq_strerror(zmqError);
        auto errorMsg = "Failed to bind to: " + endPoint + "\n"
                      + "ZMQ failed with:\n" + zmqErrorMsg;
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg); 
    }
    pImpl->mEndPoints.insert(std::pair(endPoint, true));
}
*/

/// Send a message
/*
void Publisher::send(const size_t nBytes, const char *message)
{
    if (nBytes == 0)
    {
        pImpl->mLogger->info("Message is empty - will not publish");
        return;
    }
    if (message == nullptr)
    {
        pImpl->mLogger->error("Message is NULL");
        throw std::invalid_argument("Message is NULL");
    }
    if (pImpl->mEndPoints.empty())
    {
        pImpl->mLogger->error("No end points - run bind first");
        throw std::runtime_error("No end points - run bind first");
    }
    auto zmqError = zmq_send(pImpl->mPublisher, message, nBytes, 0);
    if (zmqError != 0)
    {
        auto zmqErrorMsg = std::string(zmq_strerror(zmqError));
        auto errorMsg = "Failed to send message with error: " + zmqErrorMsg;
        pImpl->mLogger->error(errorMsg);
        throw std::runtime_error(errorMsg);
    }
}

/// Send a message
void Publisher::send(const std::string &message)
{
    send(message.size(), message.c_str()); 
}
*/


/*
/// Send a message
void Publisher::send(const MessageFormats::IMessage &message)
{
    auto messageType = message.getMessageType();
    if (messageType.empty())
    {
        pImpl->mLogger->debug("Message type is empty");
    }
    auto cborMessage = std::string(message.toCBOR());
    if (cborMessage.empty())
    {
        pImpl->mLogger->debug("CBOR message is empty");
    }
    //pImpl->mPublisher.send(zmq::buffer(messageType), zmq::send_flags::sndmore);
    //pImpl->mPublisher.send(zmq::buffer(cborMessage));
    //s_sendmore(pImpl->mPublisher, messageType.data());
    //s_send(pImpl->mPublisher, cborMessage.data());
    auto debugMessage = "Sending message type: " + messageType
                      + " with length "
                      + std::to_string(cborMessage.size());
    pImpl->mLogger->debug(debugMessage);
    auto rc = zmq_send(pImpl->mPublisher, messageType.data(),
                       messageType.size(), ZMQ_SNDMORE);
    if (rc != static_cast<int> (messageType.size()))
    {
        auto error = "Failed to send message type: " + messageType;
        pImpl->mLogger->error(error);
    }
    rc = zmq_send(pImpl->mPublisher, cborMessage.data(),
                  cborMessage.size(), 0);
    if (rc != static_cast<int> (cborMessage.size()))
    {
        auto error = "Failed to send message contents";
        pImpl->mLogger->error(error);
    }
}
*/

/*
void Publisher::send(const std::vector<uint8_t> &message)
{
    auto nBytes = message.size();
    if (nBytes == 0)
    {
        pImpl->mLogger->info("Message is empty - will not publish");
        return;
    }
    zmq::message_t zmqMessage(nBytes);
    std::memcpy(zmqMessage.data(), message.data(), nBytes);
    pImpl->mPublisher.send(zmqMessage);
}
*/
