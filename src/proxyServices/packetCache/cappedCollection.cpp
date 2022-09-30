#include <iostream>
#include <string>
#include <map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include "umps/proxyServices/packetCache/cappedCollection.hpp"
#include "umps/proxyServices/packetCache/circularBuffer.hpp"
#include "umps/logging/stdout.hpp"
#include "private/applications/packetCache.hpp"

using namespace UMPS::ProxyServices::PacketCache;

/// Implementation
template<class T>
class CappedCollection<T>::CappedCollectionImpl
{
public:
    /// C'tor
    CappedCollectionImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    /// C'tor
    explicit CappedCollectionImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// Destructor
    ~CappedCollectionImpl()
    {
        clear();
        mLogger = nullptr;
    }
    /// Reset class
    void clear() noexcept
    {
        std::scoped_lock lock(mMutex);
        mCircularBufferMap.clear();
        mMaxPackets = 0;
        mInitialized = false; 
    }
    /// Have sensor?
    [[nodiscard]] bool haveSensor(const std::string &name) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mCircularBufferMap.contains(name);
        //auto it = mCircularBufferMap.find(name);
        //return (it != mCircularBufferMap.end());
    }
    /// Get all sensor names
    //[[nodiscard]] std::vector<std::string> getSensors() const noexcept
    [[nodiscard]] std::unordered_set<std::string> getSensors() const noexcept
    {
        std::unordered_set<std::string> result;
        std::scoped_lock lock(mMutex);
        result.reserve(mCircularBufferMap.size());
        for (auto it = mCircularBufferMap.begin();
             it != mCircularBufferMap.end(); ++it)
        {
            result.insert(it->first);
        }
        /*
        for (const auto &cb : mCircularBufferMap)
        {
            result.push_back(cb.first);
        }
        */
        return result;
    }
    /// Update (with move semantics for speed - i.e., no copies)
    void update(UMPS::MessageFormats::DataPacket<T> &&packet)
    {
        auto name = makeName(packet);
        std::scoped_lock lock(mMutex);
        auto it = mCircularBufferMap.find(name);
        if (it == mCircularBufferMap.end())
        {
            mLogger->debug("Adding: " + name);
            CircularBuffer<T> cbNew;
            cbNew.initialize(packet.getNetwork(),
                             packet.getStation(),
                             packet.getChannel(),
                             packet.getLocationCode(),
                             mMaxPackets);
            cbNew.addPacket(std::move(packet));
            mCircularBufferMap.insert(std::pair(name, cbNew));
        }
        else
        {
            if (mLogger->getLevel() >= UMPS::Logging::Level::Debug)
            {
                mLogger->debug("Updating: " + name);
            }
            it->second.addPacket(std::move(packet));
        } 
    }
    /// Get total number of packets
    [[nodiscard]] int getTotalNumberOfPackets() const noexcept
    {
        int nPackets = 0;
        std::scoped_lock lock(mMutex);
        for (auto it = mCircularBufferMap.begin();
             it != mCircularBufferMap.end(); ++it)
        {
            nPackets = nPackets + it->second.getNumberOfPackets();
        }
        /*
        for (const auto &cb : mCircularBufferMap)
        {
            nPackets = nPackets + cb.second.getNumberOfPackets();
        }
        */
        return nPackets;
    }
    /// Query
    std::vector<UMPS::MessageFormats::DataPacket<T>>
        getPackets(const std::string &name, const std::chrono::microseconds &t0)
    {
        std::vector<UMPS::MessageFormats::DataPacket<T>> result;
        std::scoped_lock lock(mMutex);
        auto it = mCircularBufferMap.find(name);
        if (it != mCircularBufferMap.end())
        {
            return std::move(it->second.getPackets(t0));
        }
        throw std::invalid_argument("Sensor: " + name
                                  + " not in collection");
    }
    /// Query
    std::vector<UMPS::MessageFormats::DataPacket<T>>
        getPackets(const std::string &name,
                   const std::chrono::microseconds &t0,
                   const std::chrono::microseconds &t1) 
    {   
        std::vector<UMPS::MessageFormats::DataPacket<T>> result;
        std::scoped_lock lock(mMutex);
        auto it = mCircularBufferMap.find(name);
        if (it != mCircularBufferMap.end())
        {
            return std::move(it->second.getPackets(t0, t1));
        }
        throw std::invalid_argument("Sensor: " + name
                                  + " not in collection");
    }
    /// Get earliest start time
    std::chrono::microseconds
        getEarliestStartTime(const std::string &name) const
    {
        std::scoped_lock lock(mMutex);
        auto it = mCircularBufferMap.find(name);
        if (it != mCircularBufferMap.end())
        {   
            return it->second.getEarliestStartTime();
        }
        return std::chrono::microseconds{std::numeric_limits<int>::lowest()};
    }
    /*
    /// Get earliest start time
    std::chrono::microseconds getEarliestStartTime() const
    {
        std::chrono::microseconds earliestTime{std::numeric_limits<int>::max()};
        for (auto it = mCircularBufferMap.begin();
             it != mCircularBufferMap.end(); ++it)
        {
            earliestTime = std::min(earliestTime,
                                    it->second.getEarliestStartTime());
        }
        return earliestTime;
    }       
    */
///private:
    mutable std::mutex mMutex;
    std::map<std::string, CircularBuffer<T>> mCircularBufferMap;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    int mMaxPackets = 0;
    bool mInitialized = false;
};

/// C'tor
template<class T>
CappedCollection<T>::CappedCollection() :
    pImpl(std::make_unique<CappedCollectionImpl> ())
{
}

/// C'tor
template<class T>
CappedCollection<T>::CappedCollection(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<CappedCollectionImpl> (logger))
{
}

/// Destructor
template<class T>
CappedCollection<T>::~CappedCollection() = default;

/// Initialization
template<class T>
void CappedCollection<T>::initialize(const int maxPackets)
{
    clear();
    if (maxPackets < 1)
    {
        throw std::invalid_argument("Max number of packets = "
                                  + std::to_string(maxPackets)
                                  + " must be positive");
    }
    pImpl->mMaxPackets = maxPackets;
    pImpl->mInitialized = true;
}

/// Initialized?
template<class T>
bool CappedCollection<T>::isInitialized() const noexcept
{
    return pImpl->mInitialized;
}

/// Add a packet
template<class T>
void CappedCollection<T>::addPacket(
    const UMPS::MessageFormats::DataPacket<T> &packet)
{
    auto packetCopy = packet;
    addPacket(std::move(packetCopy));
}

/// Add packet with move
template<class T>
void CappedCollection<T>::addPacket(
    UMPS::MessageFormats::DataPacket<T> &&packet)
{
    if (!isInitialized()){throw std::runtime_error("Class not initialized");}
    if (!isValidPacket(packet))
    {
        throw std::invalid_argument("Packet is invalid");
    }
    pImpl->update(std::move(packet));
}

/// Reset the class
template<class T>
void CappedCollection<T>::clear() noexcept
{
    pImpl->clear();
}

/// Have SNCL?
template<class T>
bool CappedCollection<T>::haveSensor(
    const std::string &network, const std::string &station,
    const std::string &channel, const std::string &locationCode) const noexcept
{
    if (!isInitialized()){return false;}
    auto name = makeName(network, station, channel, locationCode);
    return haveSensor(name);
}

template<class T>
bool CappedCollection<T>::haveSensor(const std::string &name) const noexcept
{
    if (!isInitialized()){return false;}
    return pImpl->haveSensor(name);
}

/// Get all the sensor names
template<class T>
std::unordered_set<std::string>
    CappedCollection<T>::getSensorNames() const noexcept
{
    return pImpl->getSensors();
}

/// Get total number of packets
template<class T>
int CappedCollection<T>::getTotalNumberOfPackets() const noexcept
{
    return pImpl->getTotalNumberOfPackets();
}

/// Earliest time
template<class T>
std::chrono::microseconds
    CappedCollection<T>::getEarliestStartTime(const std::string &name) const
{
    auto t = pImpl->getEarliestStartTime(name);
    if (t == std::chrono::microseconds{std::numeric_limits<int>::lowest()})
    {
        throw std::runtime_error("Sensor " + name
                               + " does not exist in collection");
    }
    return t;
}


/// Get packets from t0 to now
template<class T>
std::vector<UMPS::MessageFormats::DataPacket<T>>
    CappedCollection<T>::getPackets(const std::string &name,
                                    const std::chrono::microseconds &t0) const
{
    if (!haveSensor(name))
    {
        throw std::runtime_error("Sensor " + name + " not in collection");
    }
    return pImpl->getPackets(name, t0); 
}

template<class T>
std::vector<UMPS::MessageFormats::DataPacket<T>>
    CappedCollection<T>::getPackets(const std::string &name,
                                    const double t0) const
{
    return getPackets(name, secondsToMicroSeconds(t0));
}

/// Get packets from t0 to t1 
template<class T>
std::vector<UMPS::MessageFormats::DataPacket<T>>
    CappedCollection<T>::getPackets(const std::string &name,
                                    const std::chrono::microseconds &t0,
                                    const std::chrono::microseconds &t1) const
{
    if (!haveSensor(name))
    {   
        throw std::runtime_error("Sensor " + name + " not in collection");
    }
    if (t1 <= t0)
    {
        throw std::invalid_argument("t0 = " + std::to_string(t0.count())
                                  + " must be less than t1 = "
                                  + std::to_string(t1.count()));
    }
    return pImpl->getPackets(name, t0, t1);
}

template<class T>
std::vector<UMPS::MessageFormats::DataPacket<T>>
    CappedCollection<T>::getPackets(const std::string &name,
                                    const double t0,
                                    const double t1) const
{
    return getPackets(name,
                      secondsToMicroSeconds(t0),
                      secondsToMicroSeconds(t1));
}

///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------///
template class UMPS::ProxyServices::PacketCache::CappedCollection<double>;
template class UMPS::ProxyServices::PacketCache::CappedCollection<float>;
