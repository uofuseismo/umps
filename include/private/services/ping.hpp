#ifndef PRIVATE_SERVICES_PING_HPP
#define PRIVATE_SERVICES_PING_HPP
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include "umps/messageFormats/message.hpp"
namespace
{
class PingRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// Convert class to a message
    [[nodiscard]] std::string toMessage() const final 
    {
        nlohmann::json obj;
        obj["MessageType"] = getMessageType();
        obj["MessageVersion"] = getMessageVersion();
        obj["Time"] = static_cast<int64_t> (getTime().count());
        auto v = nlohmann::json::to_cbor(obj);
        return std::string{v.begin(), v.end()};
    }
    /// Convert class from a mesage
    void fromMessage(const std::string &message) final
    {
        if (message.empty()){throw std::invalid_argument("Message is empty");}
        fromMessage(message.data(), message.size());
    }
    void fromMessage(const char *messageIn, const size_t length) final
    {
        const auto message = reinterpret_cast<const uint8_t *> (messageIn);
        auto obj = nlohmann::json::from_cbor(message, message + length);
        if (obj["MessageType"] != getMessageType())
        {
            throw std::invalid_argument("Message has invalid message type");
        }
        setTime(std::chrono::milliseconds {obj["Time"].get<int64_t> ()});
    }
    /// Sets time to now 
    void setTimeToNow()
    {
        auto now = std::chrono::high_resolution_clock::now();
        setTime(std::chrono::duration_cast<std::chrono::milliseconds> (
                       now.time_since_epoch()));
    }
    void setTime(const std::chrono::milliseconds &time) noexcept
    {
        mTime = time;
    }
    [[nodiscard]] std::chrono::milliseconds getTime() const noexcept
    {
        return mTime;
    }
    /// Copy class
    [[nodiscard]]
    std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<PingRequest> (*this);
        return result;
    } 
    /// Create an instance of this class
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        createInstance() const noexcept final
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<PingRequest> (); 
        return result;
    }
    /// Message type
    [[nodiscard]] std::string getMessageType() const noexcept final
    {
        return "::PingRequest";
    }
    /// Message version
    [[nodiscard]] std::string getMessageVersion() const noexcept
    {
        return "1.0.0";
    }
private:
    std::chrono::milliseconds mTime{
        std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::high_resolution_clock::now().time_since_epoch())
    };
};
//----------------------------------------------------------------------------//
class PingResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// Convert class to a message
    [[nodiscard]] std::string toMessage() const final
    {
        nlohmann::json obj;
        obj["MessageType"] = getMessageType();
        obj["MessageVersion"] = getMessageVersion();
        obj["Time"] = static_cast<int64_t> (getTime().count());
        auto v = nlohmann::json::to_cbor(obj);
        return std::string{v.begin(), v.end()};
    }
    /// Convert class from a mesage
    void fromMessage(const std::string &message) final
    {
        if (message.empty()){throw std::invalid_argument("Message is empty");}
        fromMessage(message.data(), message.size());
    }
    void fromMessage(const char *messageIn, const size_t length) final
    {
        const auto message = reinterpret_cast<const uint8_t *> (messageIn);
        auto obj = nlohmann::json::from_cbor(message, message + length);
        if (obj["MessageType"] != getMessageType())
        {
            throw std::invalid_argument("Message has invalid message type");
        }
        setTime(std::chrono::milliseconds {obj["Time"].get<int64_t> ()});
    }
    // Set time
    void setTime(const std::chrono::milliseconds &time) noexcept
    {
        mTime = time;
    }
    [[nodiscard]] std::chrono::milliseconds getTime() const noexcept
    {
        return mTime;
    }
    /// Copy class
    [[nodiscard]]
    std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<PingResponse> (*this);
        return result;
    }
    /// Create an instance of this class
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        createInstance() const noexcept final
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<PingResponse> ();
        return result;
    }
    /// Message type
    [[nodiscard]] std::string getMessageType() const noexcept final
    {
        return "::PingResponse";
    }
    /// Message version
    [[nodiscard]] std::string getMessageVersion() const noexcept
    {
        return "1.0.0";
    }
private:
    std::chrono::milliseconds mTime{
        std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::high_resolution_clock::now().time_since_epoch())
    };
};
}
#endif
