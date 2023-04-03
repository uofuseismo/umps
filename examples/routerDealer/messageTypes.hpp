#ifndef EXAMPLES_ROUTER_DEALER_MESSAGE_TYPES_HPP
#define EXAMPLES_ROUTER_DEALER_MESSAGE_TYPES_HPP
#include <string>
#include <umps/messageFormats/message.hpp>
#include <nlohmann/json.hpp>

namespace
{

/// This is an application specific request message.
class RequestMessage : public UMPS::MessageFormats::IMessage
{
public:
    /// Destructor
    ~RequestMessage() override = default; 
    /// Sets the message contents
    void setContents(const std::string &contents)
    {
        mContents = contents;
    }
    /// Gets the message contents
    [[nodiscard]] std::string getContents() const noexcept
    {
        return mContents;
    } 
    /// Makes a copy of this message
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        clone() const override
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<RequestMessage> (*this);    
        return result;
    } 
    /// Makes an instance of this message
    std::unique_ptr<UMPS::MessageFormats::IMessage>
         createInstance() const noexcept override  
    {
         std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<RequestMessage> ();
         return result;
    }
    /// Message version
    [[nodiscard]] std::string getMessageVersion() const noexcept override
    {
        return "1.0.0";
    }
    /// Message type
    [[nodiscard]] std::string getMessageType() const noexcept override
    {
        return "ExampleRequestMessage";
    }
    /// Serialize message
    [[nodiscard]] std::string toMessage() const override
    {
        nlohmann::json obj;
        obj["MessageType"] = getMessageType();
        obj["MessageVersion"] = getMessageVersion();
        obj["Contents"] = getContents();
        return obj.dump();
    }
    /// Deserialize message
    void fromMessage(const std::string &message) override
    {
        fromMessage(message.c_str(), message.size());
    }
    /// Deserialize message
    void fromMessage(const char *message, const size_t length) override
    {
        auto obj = nlohmann::json::parse(message, message + length);
        setContents(obj["Contents"].get<std::string> ());
    }
private:
    std::string mContents;
};

/// This is an application specific reply message.
/// For simplicity, it is exactly the same as the request message with
/// the exception of the message type.
class ReplyMessage : public UMPS::MessageFormats::IMessage
{
public:
    /// Destructor
    ~ReplyMessage() override = default;
    /// Sets the message contents
    void setContents(const std::string &contents)
    {
        mContents = contents;
    }
    /// Gets the message contents
    [[nodiscard]] std::string getContents() const noexcept
    {
        return mContents;
    }
    /// Makes a copy of this message
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        clone() const override
    {
        std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<ReplyMessage> (*this);
        return result;
    }
    /// Makes an instance of this message
    std::unique_ptr<UMPS::MessageFormats::IMessage>
         createInstance() const noexcept override
    {
         std::unique_ptr<UMPS::MessageFormats::IMessage> result
            = std::make_unique<ReplyMessage> ();
         return result;
    }
    /// Message version
    [[nodiscard]] std::string getMessageVersion() const noexcept override
    {
        return "1.0.0";
    }
    /// Message type
    [[nodiscard]] std::string getMessageType() const noexcept override
    {
        return "ExampleReplyMessage";
    }
    /// Serialize message
    [[nodiscard]] std::string toMessage() const override
    {
        nlohmann::json obj;
        obj["MessageType"] = getMessageType();
        obj["MessageVersion"] = getMessageVersion();
        obj["Contents"] = getContents();
        return obj.dump();
    }
    /// Deserialize message
    void fromMessage(const std::string &message) override
    {
        fromMessage(message.c_str(), message.size());
    }
    /// Deserialize message
    void fromMessage(const char *message, const size_t length) override
    {
        auto obj = nlohmann::json::parse(message, message + length);
        setContents(obj["Contents"].get<std::string> ());
    }
private:
    std::string mContents;

};

}
#endif
