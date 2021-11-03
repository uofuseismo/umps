#include <vector>
#include <string>
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"

using namespace UMPS::MessageFormats;

class Messages::MessagesImpl
{
public:
    /// C'tor
    MessagesImpl() = default;
    /// Copy c'tor
    MessagesImpl(const MessagesImpl &messages)
    {
        *this = messages;
    } 
    /// Copy assignment
    MessagesImpl& operator=(const MessagesImpl &messages)
    {
        if (&messages == this){return *this;}
        for (const auto &m : messages.mMessages)
        {
            mMessages.insert(std::pair(m.first, m.second->createInstance()));
        }
        return *this;
    }
///private:
    std::map<std::string, std::unique_ptr<IMessage>> mMessages;
};

/// C'tor
Messages::Messages() :
    pImpl(std::make_unique<MessagesImpl> ())
{
}

/// Copy c'tor
Messages::Messages(const Messages &messages)
{
    *this = messages;
}

/// Move c'tor
Messages::Messages(Messages &&messages) noexcept
{
    *this = std::move(messages);
}

/// Copy assignment
Messages& Messages::operator=(const Messages &messages)
{
    if (&messages == this){return *this;}
    pImpl = std::make_unique<MessagesImpl> (*messages.pImpl);
    return *this;
}

/// Move assignment
Messages& Messages::operator=(Messages &&messages) noexcept
{
    if (&messages == this){return *this;}
    pImpl = std::move(messages.pImpl);
    return *this;
}

/// Destructor
Messages::~Messages() = default;

/// Adds a messages
void Messages::add(const std::unique_ptr<IMessage> &message)
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    auto messageType = message->getMessageType();
    if (contains(messageType))
    {
        throw std::runtime_error("Message of type: " + messageType
                               + " already exists");
    }
    pImpl->mMessages.insert(std::pair(messageType, message->createInstance()));
}

/// Does this message type exist
bool Messages::contains(const std::unique_ptr<IMessage> &message) const
{
    if (message == nullptr){throw std::invalid_argument("Message is NULL");}
    auto messageType = message->getMessageType();
    return contains(messageType);
}

bool Messages::contains(const std::string &messageType) const noexcept
{
    return pImpl->mMessages.contains(messageType);
}

/// Gets all the messages 
std::map<std::string, std::unique_ptr<IMessage>>
    Messages::get() const noexcept
{
    std::map<std::string, std::unique_ptr<IMessage>> result;
    for (const auto &m : pImpl->mMessages)
    {
        result.insert(std::pair(m.first, m.second->createInstance()));
    }
    return result;
}

std::unique_ptr<IMessage> Messages::get(const std::string &messageType) const
{
    if (!contains(messageType))
    {
        throw std::invalid_argument("Message type: "
                                  + messageType + " not set");
    }
    auto it = pImpl->mMessages.find(messageType);
    return it->second->createInstance();
}
 

/// Clear
void Messages::clear() noexcept
{
    pImpl = std::make_unique<MessagesImpl> ();
}

/// Size
int Messages::size() const noexcept
{
    return static_cast<int> (pImpl->mMessages.size());
}

/// Empy?
bool Messages::empty() const noexcept
{
    return (size() == 0);
}
