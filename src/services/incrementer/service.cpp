#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <cassert>
#include "umps/services/incrementer/service.hpp"
#include "umps/messaging/requestRouter/router.hpp"
#include "umps/services/incrementer/parameters.hpp"
#include "umps/services/incrementer/counter.hpp"
#include "umps/services/incrementer/response.hpp"
#include "umps/services/incrementer/request.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Services::Incrementer;

class Service::ServiceImpl
{
public:
    /// Constructs
    ServiceImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()),
        mRouter(mLogger)
    {
    }
    explicit ServiceImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger),
        mRouter(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// The callback to handle incrementer requests
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *messageContents, const size_t length) noexcept
    {
        mLogger->debug("ServiceImpl::callback: Message of type: " + messageType 
                     + " with length: " + std::to_string(length)
                     + " bytes was received.  Processing...");
        Request request;
        auto response
            = std::make_unique<Response> ();
        if (messageType == request.getMessageType())
        {
            // Unpack the request
            try
            {
                auto cborMessageContents
                    = static_cast<const uint8_t *> (messageContents); 
                request.fromCBOR(cborMessageContents, length);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Request serialization failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::INVALID_MESSAGE);
                return response;
            }
            // Set the identifier to help out the recipient
            response->setIdentifier(request.getIdentifier());
            // Process the request
            try
            {
                auto nextValue = mCounter.getNextValue();
                response->setValue(nextValue);
                response->setReturnCode(ReturnCode::SUCCESS);
            }
            catch (const std::exception &e)
            {
                mLogger->error("Incrementer failed with: "
                             + std::string(e.what()));
                response->setReturnCode(ReturnCode::ALGORITHM_FAILURE);
            }
        }
        else
        {
            mLogger->error("Expecting message type: " + messageType
                         + " but received: " + request.getMessageType());
            response->setReturnCode(ReturnCode::NO_ITEM);
        }
        return response;
    }
/*
    /// Determines if the service was started
    bool isRunning() const noexcept
    {
        //auto running = mRunning.load(std::memory_order_relaxed);
        std::scoped_lock lock(mMutex);
        auto running = mRunning;
        return running;
    }
    /// Marks the service as running or not running
    void setRunning(const bool status)
    {
        //mRunning.store(status, std::memory_order_seq_cst);
        std::scoped_lock lock(mMutex);
        mRunning = status; 
    }
*/
///private:
    std::shared_ptr<UMPS::Logging::ILog> mLogger = nullptr;
    Counter mCounter; 
    Parameters mParameters;
    UMPS::Messaging::RequestRouter::Router mRouter;
    //mutable std::mutex mMutex;
    std::string mName;
    // Timeout in milliseconds.  0 means return immediately while -1 means
    // wait indefinitely.
    std::chrono::milliseconds mPollTimeOutMS{0};
    bool mRunning = false;
    //std::atomic<bool> mRunning{false};
    bool mInitialized = false;
};

/// C'tor
Service::Service() :
    pImpl(std::make_unique<ServiceImpl> ())
{
}

Service::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (logger))
{
}

/// Move c'tor
Service::Service(Service &&service) noexcept
{
    *this = std::move(service);
}

/// Move assignment
Service& Service::operator=(Service &&service) noexcept
{
    if (&service == this){return *this;}
    pImpl = std::move(service.pImpl);
    return *this;
}

/// Destructor
Service::~Service() = default;

/// Is the service running
bool Service::isRunning() const noexcept
{
    return pImpl->mRouter.isRunning();
}

/// Initialize
void Service::initialize(const Parameters &parameters)
{
    stop(); // Ensure the service is stopped
    // Create the counter
    auto name = parameters.getName();
    auto initialValue = parameters.getInitialValue();
    auto increment = parameters.getIncrement();
    Counter counter;
    counter.initialize(name, initialValue, increment);
    // Initialize the socket
    auto clientAccessAddress = parameters.getClientAccessAddress();
    pImpl->mRouter.bind(clientAccessAddress);
    if (!pImpl->mRouter.isBound())
    {
        throw std::runtime_error("Failed to bind to address: "
                               + clientAccessAddress);
    }
    // This service only handles request types
    std::unique_ptr<UMPS::MessageFormats::IMessage> requestType
        = std::make_unique<Request> (); 
    pImpl->mRouter.addMessageType(requestType);
    // Bind a callback function so that requests can be processed and this
    // class's counter can be incremented.
    pImpl->mRouter.setCallback(std::bind(&ServiceImpl::callback,
                                         &*this->pImpl,
                                         std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3));
    // Create the sockets
    // auto serverAccessAddress = parameters.getServerAccessAddress();
    // auto clientAccessAddress = parameters.getClientAccessAddress();
    // setServerAccessAddress(const std::string &address)
    // Move the counter to this and tag the class as initialized
    pImpl->mCounter = std::move(counter);
    pImpl->mParameters = parameters;
    pImpl->mName = name;
    pImpl->mInitialized = true;
}

/// Is the service initialized?
bool Service::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Runs the service
void Service::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("Service not initialized");
    }
    pImpl->mLogger->debug("Beginning service...");
    pImpl->mRouter.start(); // This should hang until the service is stopped
    pImpl->mLogger->debug("Thread exiting service");
/*
    pImpl->setRunning(true);
    auto name = pImpl->mCounter.getName();
    constexpr size_t nPollItems = 1; 
    zmq::pollitem_t items[] =
    {
        {pImpl->mServer->handle(), 0, ZMQ_POLLIN, 0}
    };
    while (isRunning())
    {
        zmq::poll(&items[0], nPollItems, pImpl->mPollTimeOutMS);
        if (items[0].revents & ZMQ_POLLIN)
        {
            // Get the next message
            std::vector<zmq::message_t> messagesReceived;
            zmq::recv_result_t receivedResult =
                zmq::recv_multipart(*pImpl->mServer,
                                    std::back_inserter(messagesReceived),
                                    zmq::recv_flags::none);
#ifndef NDEBUG
            assert(*receivedResult == 2);
#else
            if (*receivedResult != 2)
            {
                pImpl->mLogger->error("Only 2-part messages handled");
                //throw std::runtime_error("Only 2-part messages handled");
            }
#endif
            // Process the message 
            std::string messageType = messagesReceived.at(0).to_string();
            const auto payload
                = static_cast<uint8_t *> (messagesReceived.at(1).data());
            auto messageLength = messagesReceived.at(1).size();
            auto name = pImpl->mCounter.getName();
            Response response;
            try
            {
                Request request;
                request.fromCBOR(payload, messageLength); 
                auto id = request.getIdentifier();
                response.setIdentifier(id);
                // Ensure I'm counting the right item
                auto item = request.getItem();
                if (item != name)
                {
                    response.setValue(0);
                    response.setReturnCode(ReturnCode::NO_ITEM);
                    pImpl->mLogger->error("Request of type: " + item
                                       + " does not match this counter's item: "
                                       + name);
                }
                else
                {
                    try
                    {
                        auto nextValue = pImpl->mCounter.getNextValue();
                        response.setValue(nextValue);
                        response.setReturnCode(ReturnCode::SUCCESS);
                    }
                    catch (const std::exception &e)
                    {
                        pImpl->mLogger->error("Failed to get next counter");
                        response.setReturnCode(ReturnCode::ALGORITHM_FAILURE);
                    }
                }
            }
            catch (const std::exception &e)
            {
                pImpl->mLogger->error("Failed to unpack request: "
                                     + std::string(e.what()));
                response.setReturnCode(ReturnCode::INVALID_MESSAGE);
            }
            // Send result back
            auto cborMessage = response.toCBOR();
            zmq::const_buffer header{messageType.data(), messageType.size()};
            pImpl->mServer->send(header, zmq::send_flags::sndmore);
            zmq::const_buffer buffer{cborMessage.data(), cborMessage.size()};
            pImpl->mServer->send(buffer);
        }
    }
    pImpl->mLogger->debug("Thread exiting service");
*/
}

/// Gets the service name
std::string Service::getName() const
{
    if (!isInitialized()){throw std::runtime_error("Service not initialized");}
    return pImpl->mName;
}

/// Gets the request address
std::string Service::getRequestAddress() const
{
    if (!isRunning()){throw std::runtime_error("Service is not running");}
    return pImpl->mParameters.getClientAccessAddress();
}

/// Stop the service
void Service::stop()
{
    pImpl->mRouter.stop();
    //pImpl->mCounter.reset();
}
