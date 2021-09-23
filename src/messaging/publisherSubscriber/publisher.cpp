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
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
            mPublisher = std::make_unique<zmq::socket_t> (*mContext,
                                                         zmq::socket_type::pub);
        }
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
        if (context == nullptr)
        {
            mContext = std::make_shared<zmq::context_t> (1);
            mPublisher = std::make_unique<zmq::socket_t> (*mContext,
                                                         zmq::socket_type::pub);
        }
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
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
    //auto cborMessage = std::string(message.toCBOR());
    auto cborMessage = message.toMessage();
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

