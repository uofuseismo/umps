#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <cassert>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "urts/modules/incrementer/service.hpp"
#include "urts/messaging/requestRouter/router.hpp"
#include "urts/modules/incrementer/parameters.hpp"
#include "urts/modules/incrementer/counter.hpp"
#include "urts/modules/incrementer/response.hpp"
#include "urts/modules/incrementer/request.hpp"
#include "urts/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace URTS::Modules::Incrementer;

class Service::ServiceImpl
{
public:
    /// Constructs
    ServiceImpl() :
        mContext(std::make_shared<zmq::context_t> (1)),
        mServer(std::make_unique<zmq::socket_t> (*mContext,
                                                 zmq::socket_type::router)),
        mLogger(std::make_shared<URTS::Logging::StdOut> ())
    {
    }
    /// The callback to handle requests
    std::unique_ptr<URTS::MessageFormats::IMessage>
        callback(const URTS::MessageFormats::IMessage *messagePtr)
    {
        Response response;
        auto messageType = messagePtr->getMessageType();
        Request request;
        if (request.getMessageType() == messageType)
        {
            auto requestPtr = reinterpret_cast<const Request *> (messagePtr);
            auto item = requestPtr->getItem(); 
            if (item == mName)
            {

            }
            else
            {
                mLogger->error("Increment request for: " + item
                             + " does not match this counter's item: " + mName);
            }
            //auto value = mCounter.getNextValue(); 
            //request.setValue(value);
        } 
        else
        {
            mLogger->error("Unknown message type: " + messageType);
        }
        return response.clone();
    }
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
///private:
    std::shared_ptr<zmq::context_t> mContext;
    std::unique_ptr<zmq::socket_t> mServer;
    std::shared_ptr<URTS::Logging::ILog> mLogger = nullptr;
    Counter mCounter; 
    URTS::Messaging::RequestRouter::Router mRouter;
    mutable std::mutex mMutex;
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

/// Destructor
Service::~Service() = default;

/// Is the service running
bool Service::isRunning() const noexcept
{
    return pImpl->isRunning();
}

/// Initialize
void Service::initialize(const Parameters &parameters)
{
    stop();
    // Create the counter
    auto name = parameters.getName();
    auto initialValue = parameters.getInitialValue();
    auto increment = parameters.getIncrement();
    Counter counter;
    counter.initialize(name, initialValue, increment);
    // Create the sockets
    auto serverAccessAddress = parameters.getServerAccessAddress();
    auto clientAccessAddress = parameters.getClientAccessAddress();
//    setServerAccessAddress(const std::string &address)
    // Move the counter to this and tag the class as initialized
    pImpl->mCounter = std::move(counter);
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
}

/// Stop the service
void Service::stop()
{
    pImpl->setRunning(false);  // This will release the start thread
}
