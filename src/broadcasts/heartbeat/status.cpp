#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include <boost/asio/ip/host_name.hpp>
#include "umps/broadcasts/heartbeat/status.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "UMPS::Broadcasts::Heartbeat::Status"

using namespace UMPS::Broadcasts::Heartbeat;

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
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm utc_tm = *gmtime(&tt);
    char cDate[44];
    std::fill(cDate, cDate + 44, '\0');
    sprintf(cDate, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
            utc_tm.tm_year + 1900,
            utc_tm.tm_mon + 1,
            utc_tm.tm_mday,
            utc_tm.tm_hour,
            utc_tm.tm_min,
            utc_tm.tm_sec,
            static_cast<int> (ms.count()));
    return std::string(cDate);
}

nlohmann::json toJSONObject(const Status &status)
{
    nlohmann::json obj;
    obj["MessageType"] = status.getMessageType();
    obj["Module"] = status.getModule();
    obj["HostName"] = status.getHostName();
    obj["ModuleStatus"] = static_cast<int> (status.getModuleStatus());
    obj["TimeStamp"] = status.getTimeStamp();
    return obj;
}

Status objectToStatus(const nlohmann::json &obj)
{
    Status status;
    if (obj["MessageType"] != status.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    status.setModule(obj["Module"].get<std::string> ());
    status.setHostName(obj["HostName"].get<std::string> ());
    status.setModuleStatus(static_cast<ModuleStatus> (obj["ModuleStatus"]));
    status.setTimeStamp(obj["TimeStamp"].get<std::string> ());
    return status;
}

Status fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToStatus(obj);
}

Status fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToStatus(obj);
}

}

///--------------------------------------------------------------------------///
///                            Implementation                                ///
///--------------------------------------------------------------------------///
class Status::StatusImpl
{
public:
    std::string mModule = "unknown";
    std::string mHostName = boost::asio::ip::host_name(); 
    std::string mTimeStamp = "1970:01:01T00:00:00.000";
    ModuleStatus mStatus = ModuleStatus::UNKNOWN;
};

/// C'tor
Status::Status() :
    pImpl(std::make_unique<StatusImpl> ())
{
    setTimeStampToNow();
}

/// Copy c'tor
Status::Status(const Status &status)
{
    *this = status;
}

/// Move c'tor
Status::Status(Status &&status) noexcept
{
    *this = std::move(status);
}

/// Copy assignment
Status& Status::operator=(const Status &status)
{
    if (&status == this){return *this;}
    pImpl = std::make_unique<StatusImpl> (*status.pImpl);
    return *this;
}

/// Move assignment
Status& Status::operator=(Status &&status) noexcept
{
    if (&status == this){return *this;}
    pImpl = std::move(status.pImpl);
    return *this;
}

/// Destructor
Status::~Status() = default;

/// Reset class
void Status::clear() noexcept
{
    pImpl = std::make_unique<StatusImpl> ();
}

/// Message type
std::string Status::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}

/// Status
void Status::setModuleStatus(const ModuleStatus status) noexcept
{
    pImpl->mStatus = status;
}

ModuleStatus Status::getModuleStatus() const noexcept
{
    return pImpl->mStatus;
}

/// Module name
void Status::setModule(const std::string &module)
{
    if (isEmpty(module)){throw std::invalid_argument("Module is empty");}
    pImpl->mModule = module;
}

std::string Status::getModule() const noexcept
{
    return pImpl->mModule;
}

/// Host name
void Status::setHostName(const std::string &hostName)
{
    if (isEmpty(hostName))
    {
        throw std::invalid_argument("The host name is empty");
    }
    pImpl->mHostName = hostName;
}

std::string Status::getHostName() const noexcept
{
    return pImpl->mHostName;
}

/// Time stamp
void Status::setTimeStampToNow() noexcept
{
    setTimeStamp(createTimeStamp());
}

void Status::setTimeStamp(const std::string &timeStamp)
{
    if (isEmpty(timeStamp)){throw std::invalid_argument("Time stamp is empty");}
    if (static_cast<int> (timeStamp.size()) != 23)
    {
        throw std::invalid_argument("Time stamp must be length at least 23");
    }
    double sec;
    int year, month, dom, hour, minute, second, milliSecond;
    sscanf(timeStamp.c_str(), "%04d-%02d-%02dT%02d:%02d:%lf",
           &year,
           &month,
           &dom,
           &hour,
           &minute,
           &sec);
    second = static_cast<int> (sec);
    milliSecond = static_cast<int> (std::round((sec - second)*1000));
    if (milliSecond == 1000){milliSecond = milliSecond - 1;}
    if (month < 1 || month > 12)
    {
        throw std::invalid_argument("Month = " + std::to_string(month)
                                  + " must be in range [1,12]");
    }
    if (dom < 1 || dom > 31)
    {
        throw std::invalid_argument("Day of month = " + std::to_string(dom)
                                  + " must be in range [1,31]");
    }
    if (hour < 0 || hour > 23)
    {
        throw std::invalid_argument("Hour = " + std::to_string(hour)
                                  + " must be in range [0,23]");
    }
    if (minute < 0 || minute > 59)
    {
        throw std::invalid_argument("Minute = " + std::to_string(minute)
                                  + " must be in range [0,59]");
    }
    if (second < 0 || second > 59)
    {
        throw std::invalid_argument("Second = " + std::to_string(second)
                                  + " must be in range [0,59]");
    }
    if (milliSecond < 0 || milliSecond > 999)
    {
        throw std::invalid_argument("Millisecond = "
                                  + std::to_string(milliSecond)
                                  + " must be in range [0,999]");
    }
    pImpl->mTimeStamp = timeStamp;
}

std::string Status::getTimeStamp() const noexcept
{
    return pImpl->mTimeStamp;
} 

/// Create JSON
std::string Status::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string Status::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

/// From JSON
void Status::fromJSON(const std::string &message)
{   
    *this = fromJSONMessage(message);
}

/// From CBOR
void Status::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void Status::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {   
        throw std::invalid_argument("data is NULL");
    }   
    *this = fromCBORMessage(data, length);
}

///  Convert message
std::string Status::toMessage() const
{
    return toCBOR();
}

void Status::fromMessage(const char *messageIn, const size_t length)
{
    auto message = reinterpret_cast<const uint8_t *> (messageIn);
    fromCBOR(message, length);
}


/// Copy this class
std::unique_ptr<UMPS::MessageFormats::IMessage> Status::clone() const
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> result
        = std::make_unique<Broadcasts::Heartbeat::Status> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<UMPS::MessageFormats::IMessage>
    Status::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<Broadcasts::Heartbeat::Status> (); 
    return result;
}

/// Compare status's based on time 
bool UMPS::Broadcasts::Heartbeat::operator>(const Status &lhs,
                                            const Status &rhs)
{
    auto t1 = lhs.getTimeStamp();
    auto t2 = rhs.getTimeStamp(); 
    double sec1;
    int year1, month1, dom1, hour1, minute1, second1, milliSecond1;
    sscanf(t1.c_str(), "%04d-%02d-%02dT%02d:%02d:%lf",
           &year1,
           &month1,
           &dom1,
           &hour1,
           &minute1,
           &sec1);
    second1 = static_cast<int> (sec1);
    milliSecond1 = static_cast<int> (std::round((sec1 - second1)*1000));
    if (milliSecond1 == 1000){milliSecond1 = milliSecond1 - 1;}

    double sec2;
    int year2, month2, dom2, hour2, minute2, second2, milliSecond2;
    sscanf(t2.c_str(), "%04d-%02d-%02dT%02d:%02d:%lf",
           &year2,
           &month2,
           &dom2,
           &hour2,
           &minute2,
           &sec2);
    second2 = static_cast<int> (sec2);
    milliSecond2 = static_cast<int> (std::round((sec2 - second2)*1000));
    if (milliSecond2 == 1000){milliSecond2 = milliSecond2 - 1;} 

    if (year1 > year2){return true;}
    if (month1 > month2){return true;}
    if (dom1 > dom2){return true;}
    if (hour1 > hour2){return true;}
    if (minute1 > minute2){return true;}
    if (second1 > second2){return true;}
    if (milliSecond1 > milliSecond2){return true;}
    return false;
}
