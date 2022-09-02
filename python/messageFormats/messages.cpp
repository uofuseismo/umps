#include <vector>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "messageFormats/messages.hpp"
#include "messageFormats/message.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"
#include "initialize.hpp"

using namespace PUMPS::MessageFormats;

/// C'tor
Messages::Messages() :
    pImpl(std::make_unique<UMPS::MessageFormats::Messages> ())
{
}

/// Copy c'tor
Messages::Messages(const Messages &messages)
{
    *this = messages;
}

/// Copy c'tor
Messages::Messages(const UMPS::MessageFormats::Messages &messages)
{
    *this = messages; 
}

/// Move c'tor
Messages::Messages(Messages &&messages) noexcept
{
    *this = std::move(messages);
}

/// Native class
UMPS::MessageFormats::Messages Messages::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Destructor
Messages::~Messages() = default;

/// Copy assignment
Messages& Messages::operator=(const Messages &messages)
{
    if (&messages == this){return *this;}
    pImpl = std::make_unique<UMPS::MessageFormats::Messages> (*messages.pImpl);
    return *this;
}

Messages& Messages::operator=(const UMPS::MessageFormats::Messages &messages)
{
    pImpl = std::make_unique<UMPS::MessageFormats::Messages> (messages);
    return *this;
}

/// Move assignment
Messages& Messages::operator=(Messages &&messages) noexcept
{
    if (&messages == this){return *this;}
    pImpl = std::move(messages.pImpl);
    return *this;
}

/// Size
int Messages::size() const noexcept
{
    return pImpl->size();
}

/// Empty?
bool Messages::empty() const noexcept
{
    return pImpl->empty();
}

/// Add
void Messages::add(const IMessage &message) //std::unique_ptr<IMessage> &message)
{
     auto nativeClass = message.getBaseClass();
     pImpl->add(nativeClass); 
     auto messageName = nativeClass->getMessageType(); // Throws
     mPythonMessageTypes.push_back(message.createInstance());
}

/// Get the python message types
/*
std::map<std::string, std::unique_ptr<IMessage>>
    Messages::getPythonMessageTypes() const
{
    return mPythonMessageTypes;
}
*/

/// Gets a message
/*
std::unique_ptr<IMessage> Messages::get(const std::string &messageType) const
{
    auto message = pImpl->get(messageType);
    return message;
}
*/

void PUMPS::MessageFormats::initializeMessages(pybind11::module &m) 
{
    pybind11::class_<PUMPS::MessageFormats::Messages> o(m, "Messages");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This is a container for different types of messages.

Properties:

   size : The number of different message types.
   empty : True indicates there are no message types.
)"""";

    o.def("add",
          &Messages::add,
          "Adds a message type to the container.");
    o.def_property_readonly("size",
                            &Messages::size);
    o.def_property_readonly("empty",
                            &Messages::empty);
}
