#include "umps/messageFormats/message.hpp"

using namespace UMPS::MessageFormats;

/// Destructor
IMessage::~IMessage() = default;

/// Sets a message from a string container
void IMessage::fromMessage(const std::string &message)
{
    if (message.empty()){throw std::invalid_argument("Message is empty");}
    fromMessage(message.data(), message.size());   
}
