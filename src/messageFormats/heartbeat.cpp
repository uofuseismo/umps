#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include <boost/asio/ip/host_name.hpp>
#include "umps/messageFormats/heartbeat.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::MessageFormats::Heartbeat"

using namespace UMPS::MessageFormats;

namespace
{

std::string createTimeStamp()
{
    typedef std::chrono::duration<int,
        std::ratio_multiply<std::chrono::hours::period,
        std::ratio<24> >::type> days;
    auto now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    auto d = std::chrono::duration_cast<days>(tp);
    tp -= d;
    auto h = std::chrono::duration_cast<std::chrono::hours> (tp);
    tp -= h;
    auto m = std::chrono::duration_cast<std::chrono::minutes> (tp);
    tp -= m;
    auto s = duration_cast<std::chrono::seconds> (tp);
    tp -= s;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (tp);
    tp -= ms;
    //std::cout << d.count() << "d " << h.count() << ':'
    //          << m.count() << ':' << s.count();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm utc_tm = *gmtime(&tt);
    char cDate[40];
    std::fill(cDate, cDate + 40, '\0');
    sprintf(cDate, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            utc_tm.tm_year + 1900,
            utc_tm.tm_mon + 1,
            utc_tm.tm_mday,
            utc_tm.tm_hour,
            utc_tm.tm_min,
            utc_tm.tm_sec,
            static_cast<int> (ms.count()));
    return std::string(cDate);
}

nlohmann::json toJSONObject(const Heartbeat &heartbeat)
{
    nlohmann::json obj;
    obj["MessageType"] = heartbeat.getMessageType();
    obj["HostName"] = heartbeat.getHostName();
    obj["Status"] = static_cast<int> (heartbeat.getStatus());
    obj["TimeStamp"] = heartbeat.getTimeStamp();
    return obj;
}

Heartbeat objectToHeartbeat(const nlohmann::json obj)
{
    Heartbeat heartbeat;
    if (obj["MessageType"] != heartbeat.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Essential stuff
    heartbeat.setHostName(obj["HostName"].get<std::string> ());
    heartbeat.setStatus(static_cast<HeartbeatStatus> (obj["Status"]));
/*
    pick.setStation(obj["Station"].get<std::string> ());
    pick.setChannel(obj["Channel"].get<std::string> ());
    pick.setLocationCode(obj["LocationCode"].get<std::string> ());
    pick.setTime(obj["Time"].get<double> ());
    pick.setIdentifier(obj["Identifier"].get<uint64_t> ());
    // Optional stuff
    pick.setPolarity(
        static_cast<::Polarity> (obj["Polarity"].get<int> ()));
    if (!obj["PhaseHint"].is_null())
    {   
        pick.setPhaseHint(obj["PhaseHint"].get<std::string> ());
    }   
    if (!obj["Algorithm"].is_null())
    {   
        pick.setAlgorithm(obj["Algorithm"].get<std::string> ());
    }   
*/
    return heartbeat;
}

Heartbeat fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToHeartbeat(obj);
}

Heartbeat fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToHeartbeat(obj);
}

}

class Heartbeat::HeartbeatImpl
{
public:
    std::string mHostName = boost::asio::ip::host_name(); 
    std::string mTimeStamp = createTimeStamp();
    HeartbeatStatus mStatus = HeartbeatStatus::UNKNOWN;
};

/// C'tor
Heartbeat::Heartbeat() :
    pImpl(std::make_unique<HeartbeatImpl> ())
{
}

/// Copy c'tor
Heartbeat::Heartbeat(const Heartbeat &heartbeat)
{
    *this = heartbeat;
}

/// Move c'tor
Heartbeat::Heartbeat(Heartbeat &&heartbeat) noexcept
{
    *this = std::move(heartbeat);
}

/// Copy assignment
Heartbeat& Heartbeat::operator=(const Heartbeat &heartbeat)
{
    if (&heartbeat == this){return *this;}
    pImpl = std::make_unique<HeartbeatImpl> (*heartbeat.pImpl);
    return *this;
}

/// Move assignment
Heartbeat& Heartbeat::operator=(Heartbeat &&heartbeat) noexcept
{
    if (&heartbeat == this){return *this;}
    pImpl = std::move(heartbeat.pImpl);
    return *this;
}

/// Destructor
Heartbeat::~Heartbeat() = default;

/// Reset class
void Heartbeat::clear() noexcept
{
    pImpl = std::make_unique<HeartbeatImpl> ();
}

/// Message type
std::string Heartbeat::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Status
void Heartbeat::setStatus(const HeartbeatStatus status) noexcept
{
    pImpl->mStatus = status;
}

HeartbeatStatus Heartbeat::getStatus() const noexcept
{
    return pImpl->mStatus;
}

/// Host name
void Heartbeat::setHostName(const std::string &hostName)
{
    if (isEmpty(hostName))
    {
        throw std::invalid_argument("The host name is empty");
    }
    pImpl->mHostName = hostName;
}

std::string Heartbeat::getHostName() const noexcept
{
    return pImpl->mHostName;
}

/// Time stamp
void Heartbeat::setTimeStampToNow() noexcept
{
    setTimeStamp(createTimeStamp());
}

void Heartbeat::setTimeStamp(const std::string &timeStamp)
{
    if (isEmpty(timeStamp)){throw std::invalid_argument("Time stamp is empty");}
    pImpl->mTimeStamp = timeStamp;
}

std::string Heartbeat::getTimeStamp() const noexcept
{
    return pImpl->mTimeStamp;
} 

/// Create JSON
std::string Heartbeat::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string Heartbeat::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

/// From JSON
void Heartbeat::fromJSON(const std::string &message)
{   
    *this = fromJSONMessage(message);
}

/// From CBOR
void Heartbeat::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void Heartbeat::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string Heartbeat::toMessage() const
{
    return toCBOR();
}

void Heartbeat::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> Heartbeat::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Heartbeat> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Heartbeat::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Heartbeat> (); 
    return result;
}
