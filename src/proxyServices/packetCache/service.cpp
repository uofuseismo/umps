#include <iostream>
#include <mutex>
#include <thread>
#include <zmq.hpp>
#include "umps/proxyServices/packetCache/service.hpp"
#include "umps/proxyServices/packetCache/replier.hpp"
#include "umps/proxyServices/packetCache/replierOptions.hpp"
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriber.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/logging/stdout.hpp"
#include "private/threadSafeQueue.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UDataPacket = UMPS::ProxyBroadcasts::DataPacket;
namespace UMF = UMPS::MessageFormats;

template<class T>
class Service<T>::ServiceImpl
{
public:
    ServiceImpl(std::shared_ptr<UMPS::Logging::ILog> logger) 
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mContext = std::make_shared<zmq::context_t> ();
        mDataPacketSubscriber = std::make_unique<UDataPacket::Subscriber<T>>
                                (mContext, mLogger);
        mCappedCollection = std::make_shared<CappedCollection<T>> (mLogger);
        mPacketCacheReplier = std::make_unique<Replier<T>> (mContext, mLogger);
    }
    /// @brief A thread running this function will read messages off the queue
    ///        and put them into the queue.
    void getPackets()
    {
        while (keepRunning())
        {
            try
            {
                auto dataPacket = mDataPacketSubscriber->receive();
                if (dataPacket == nullptr){continue;} // Time out
                mDataPacketQueue.push(std::move(*dataPacket));
            }
            catch (const std::exception &e)
            {
                mLogger->error("Error receiving packet: "
                             + std::string(e.what()));
                continue;
            }
        }
        mLogger->debug("Broadcast to queue thread has exited");
    }
    /// @brief A thread running this function will read messages from the queue
    ///        and put them into the circular buffer.
    void queueToPacketCache()
    {
        while (keepRunning())
        {
            UMF::DataPacket<T> dataPacket;
            if (mDataPacketQueue.wait_until_and_pop(&dataPacket))
            {
                if (dataPacket.getNumberOfSamples() > 0)
                {
                    try
                    {
                        mCappedCollection->addPacket(std::move(dataPacket));
                    }
                    catch (const std::exception &e)
                    {
                        mLogger->error("Failed to add packet:\n"
                                     + std::string{e.what()});
                    }
                }
            }
        }
        mLogger->debug("Queue to circular buffer thread has exited");
    }
    /// @brief Starts the service
    void start()
    {
        // Start thread to read messages from broadcast and put into queue
        mLogger->debug("Starting data packet subscriber thread...");
        mDataPacketSubscriberThread
           = std::thread(&ServiceImpl::getPackets, this);
        // Start thread to read messages from queue and put into packetCache 
        mLogger->debug("Starting queue to packetCache thread...");
        mQueueToPacketCacheThread
           = std::thread(&ServiceImpl::queueToPacketCache, this);
        // Start thread to read / respond to messages
        mLogger->debug("Starting replier service...");
        mPacketCacheReplier->start();
    }
    /// @result True indicates the data packet subscriber should keep receiving
    ///         messages and putting the results in the circular buffer.
    [[nodiscard]] bool keepRunning() const
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        return mKeepRunning;
    }
    /// @brief Toggles this as running or not running
    void setRunning(const bool running)
    {
        std::lock_guard<std::mutex> lockGuard(mMutex);
        mKeepRunning = running;
    }
    /// @brief Stops the threads
    void stop()
    {
        mLogger->debug("PacketCache stopping threads...");
        setRunning(false);
        if (mPacketCacheReplier != nullptr)
        {
            if (mPacketCacheReplier->isRunning()){mPacketCacheReplier->stop();}
        }
        if (mDataPacketSubscriberThread.joinable())
        {
            mDataPacketSubscriberThread.join();
        }
        if (mQueueToPacketCacheThread.joinable())
        {
            mQueueToPacketCacheThread.join();
        }
        if (mResponseThread.joinable())
        {
            mResponseThread.join();
        }
    }
    /// Destructor
    ~ServiceImpl()
    {
        stop();
    }
//private:
    mutable std::mutex mMutex;
    std::thread mDataPacketSubscriberThread;
    std::thread mQueueToPacketCacheThread;
    std::thread mResponseThread;
    std::shared_ptr<zmq::context_t> mContext;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::unique_ptr<UDataPacket::Subscriber<T>> mDataPacketSubscriber{nullptr};
    std::shared_ptr<CappedCollection<T>> mCappedCollection{nullptr};
    std::unique_ptr<Replier<T>> mPacketCacheReplier{nullptr};
    ThreadSafeQueue<UMPS::MessageFormats::DataPacket<T>> mDataPacketQueue;
    bool mKeepRunning = true;
    bool mInitialized = false;
};

/// C'tor
template<class T>
Service<T>::Service() :
    pImpl(std::make_unique<ServiceImpl> (nullptr))
{
}

/// C'tor w/ logger
template<class T>
Service<T>::Service(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<ServiceImpl> (logger))
{
}

/// Destructor
template<class T>
Service<T>::~Service() = default;

/// Initialize the class
template<class T>
void Service<T>::initialize(
    const int maxPackets,
    const UDataPacket::SubscriberOptions<T> &dataPacketSubscriberOptions,
    const ReplierOptions &packetCacheReplierOptions)
{
    stop(); // Ensure the service is stopped
    if (maxPackets <= 0)
    {
        throw std::invalid_argument("Max packets must be positive");
    }
    pImpl->mDataPacketSubscriber->initialize(dataPacketSubscriberOptions);
    pImpl->mCappedCollection->initialize(maxPackets);
    pImpl->mPacketCacheReplier->initialize(packetCacheReplierOptions,
                                           pImpl->mCappedCollection);
    pImpl->mInitialized = pImpl->mDataPacketSubscriber->isInitialized() &&
                          pImpl->mPacketCacheReplier->isInitialized() &&
                          pImpl->mCappedCollection->isInitialized();
}

/// Start service
template<class T>
void Service<T>::start()
{
    if (!isInitialized())
    {
        throw std::runtime_error("PacketCache service not initialized");
    }
    pImpl->mLogger->debug("Starting service...");
    pImpl->start();
}

/// Running?
template<class T>
bool Service<T>::isRunning() const noexcept
{
    return pImpl->keepRunning();
}

/// Stop service
template<class T>
void Service<T>::stop()
{
    pImpl->mLogger->debug("Stopping service...");
    pImpl->stop();
}

/// Initialized?
template<class T>
bool Service<T>::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Get total number of packets
template<class T>
int Service<T>::getTotalNumberOfPackets() const noexcept
{
    return pImpl->mCappedCollection->getTotalNumberOfPackets();
}

///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyServices::PacketCache::Service<double>;
template class UMPS::ProxyServices::PacketCache::Service<float>;
