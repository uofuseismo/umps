#include <map>
#include <umps/messageFormats/message.hpp>
#include <umps/messageFormats/messages.hpp>
#include <umps/messageFormats/text.hpp>
#include <umps/messageFormats/failure.hpp>
#include <umps/messageFormats/staticUniquePointerCast.hpp>
#include <pybind11/pybind11.h>
#include "python/messageFormats.hpp"

using namespace UMPS::Python::MessageFormats;
namespace UMF = UMPS::MessageFormats;

///--------------------------------------------------------------------------///
///                                    IMessage                              ///
///--------------------------------------------------------------------------///
IMessage::~IMessage() = default;

void IMessage::fromBaseClass(UMPS::MessageFormats::IMessage &)
{
}

std::unique_ptr<IMessage> IMessage::clone(
    const std::unique_ptr<UMPS::MessageFormats::IMessage> &) const
{
    return nullptr;
}

std::unique_ptr<IMessage> IMessage::createInstance() const
{
    return nullptr;
}

std::unique_ptr<UMPS::MessageFormats::IMessage>
    IMessage::getInstanceOfBaseClass() const noexcept
{
    return nullptr;
}

std::string IMessage::getMessageType() const noexcept
{
    return "UNDEFINED_MESSAGE_TYPE";
}
///--------------------------------------------------------------------------///
///                                   Messages                               ///
///--------------------------------------------------------------------------///
/// Constructor
Messages::Messages() :
    pImpl(std::make_unique<UMPS::MessageFormats::Messages> ())
{
}

/// Copy constructor
Messages::Messages(const Messages &messages)
{
    *this = messages;
}

/// Copy constructor
Messages::Messages(const UMPS::MessageFormats::Messages &messages)
{
    *this = messages; 
}

/// Move constructor
Messages::Messages(Messages &&messages) noexcept
{
    *this = std::move(messages);
}

/// Native class
UMPS::MessageFormats::Messages Messages::getNativeClass() const noexcept
{
    return *pImpl;
}

const UMPS::MessageFormats::Messages&
    Messages::getNativeClassReference() const noexcept
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
void Messages::add(const IMessage &message)
{
    auto instance = message.createInstance();
    if (instance == nullptr){throw std::invalid_argument("Instance is NULL");}
    auto nativeClass = message.getInstanceOfBaseClass();
    pImpl->add(nativeClass); 
    //mPythonMessageTypes.push_back(std::move(instance));
}

/// Contains?
bool Messages::contains(const std::string &message) const noexcept
{
    return pImpl->contains(message);
}

/// Get the message types
std::map<std::string, IMessage> Messages::getAll() const noexcept
{
    std::map<std::string, IMessage> result;
    auto messages = pImpl->get();
    for (const auto &message : messages)
    {
        IMessage pyMessage;
        result.insert(
            std::pair{message.first,
                      *pyMessage.clone(message.second->clone())});
    }
    return result;
}

///--------------------------------------------------------------------------///
///                                 Failure Message                          ///
///--------------------------------------------------------------------------///
/// Constructor
Failure::Failure() :
    pImpl(std::make_unique<UMPS::MessageFormats::Failure> ())
{
}

Failure::Failure(const Failure &message)
{
    *this = message;
}

Failure::Failure(const UMPS::MessageFormats::Failure &message)
{
    *this = message;
}

Failure::Failure(Failure &&message) noexcept
{
    *this = std::move(message);
}

/// Operators
Failure& Failure::operator=(const Failure &message)
{
    if (&message == this){return *this;}
    pImpl = std::make_unique<UMPS::MessageFormats::Failure> (*message.pImpl);
    return *this;
}

Failure& Failure::operator=(const UMPS::MessageFormats::Failure &message)
{
    pImpl = std::make_unique<UMPS::MessageFormats::Failure> (message);
    return *this;
}

Failure& Failure::operator=(Failure &&message) noexcept
{
    if (&message == this){return *this;}
    pImpl = std::move(message.pImpl);
    return *this;
}

const UMPS::MessageFormats::Failure&
Failure::getNativeClassReference() const noexcept
{
    return *pImpl;
}

UMPS::MessageFormats::Failure Failure::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Things to override
void Failure::fromBaseClass(UMPS::MessageFormats::IMessage &message)
{
    if (message.getMessageType() != pImpl->getMessageType())
    {
        throw std::invalid_argument("Expecting message type: "
                                    + pImpl->getMessageType()
                                    + " but given: "
                                    + message.getMessageType());
    }
    pImpl = UMF::static_unique_pointer_cast<UMPS::MessageFormats::Failure>
            (message.clone());
}
std::unique_ptr<IMessage> Failure::clone(
    const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const
{
   if (message->getMessageType() != pImpl->getMessageType())
   {   
       throw std::invalid_argument("Expecting: " + pImpl->getMessageType()
                                 + " but got: " + message->getMessageType());
   }
   auto copy = UMF::static_unique_pointer_cast<UMPS::MessageFormats::Failure>
               (message->clone());
   return std::make_unique<Failure> (*copy);
}

std::unique_ptr<IMessage> Failure::createInstance() const
{
    auto result = std::make_unique<Failure> ();
    return result;
}

std::unique_ptr<UMPS::MessageFormats::IMessage>
    Failure::getInstanceOfBaseClass() const noexcept
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> message
        = pImpl->createInstance();
    return message;
}

std::string Failure::getMessageType() const noexcept
{
    return pImpl->getMessageType();
}

/// Details
void Failure::setDetails(const std::string &details) noexcept
{
    pImpl->setDetails(details);
}

std::string Failure::getDetails() const noexcept
{
    return pImpl->getDetails();
}

/// Destructors
void Failure::clear() noexcept
{
    pImpl->clear();
}

Failure::~Failure() = default;

///--------------------------------------------------------------------------///
///                                 Failure Message                          ///
///--------------------------------------------------------------------------///
/// Constructor
Text::Text() :
    pImpl(std::make_unique<UMPS::MessageFormats::Text> ())
{
}

Text::Text(const Text &message)
{
    *this = message;
}

Text::Text(const UMPS::MessageFormats::Text &message)
{
    *this = message;
}

Text::Text(Text &&message) noexcept
{
    *this = std::move(message);
}

/// Operators
Text& Text::operator=(const Text &message)
{
    if (&message == this){return *this;}
    pImpl = std::make_unique<UMPS::MessageFormats::Text> (*message.pImpl);
    return *this;
}

Text& Text::operator=(const UMPS::MessageFormats::Text &message)
{
    pImpl = std::make_unique<UMPS::MessageFormats::Text> (message);
    return *this;
}

Text& Text::operator=(Text &&message) noexcept
{
    if (&message == this){return *this;}
    pImpl = std::move(message.pImpl);
    return *this;
}

const UMPS::MessageFormats::Text&
Text::getNativeClassReference() const noexcept
{
    return *pImpl;
}

UMPS::MessageFormats::Text Text::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Things to override
void Text::fromBaseClass(UMPS::MessageFormats::IMessage &message)
{
    if (message.getMessageType() != pImpl->getMessageType())
    {
        throw std::invalid_argument("Expecting message type: "
                                    + pImpl->getMessageType()
                                    + " but given: "
                                    + message.getMessageType());
    }
    pImpl = UMF::static_unique_pointer_cast<UMPS::MessageFormats::Text>
            (message.clone());
}

std::unique_ptr<IMessage> Text::clone(
        const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const
{
    if (message->getMessageType() != pImpl->getMessageType())
    {
        throw std::invalid_argument("Expecting: " + pImpl->getMessageType()
                                    + " but got: " + message->getMessageType());
    }
    auto copy = UMF::static_unique_pointer_cast<UMPS::MessageFormats::Text>
            (message->clone());
    return std::make_unique<Text> (*copy);
}

std::unique_ptr<IMessage> Text::createInstance() const
{
    return std::make_unique<Text> ();
}

std::unique_ptr<UMPS::MessageFormats::IMessage>
Text::getInstanceOfBaseClass() const noexcept
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> message
            = pImpl->createInstance();
    return message;
}

std::string Text::getMessageType() const noexcept
{
    return pImpl->getMessageType();
}

/// Details
void Text::setContents(const std::string &contents) noexcept
{
    pImpl->setContents(contents);
}

std::string Text::getContents() const noexcept
{
    return pImpl->getContents();
}

/// Destructors
void Text::clear() noexcept
{
    pImpl->clear();
}

Text::~Text() = default;

///--------------------------------------------------------------------------///
///                                  Iniitalize                              ///
///--------------------------------------------------------------------------///
void UMPS::Python::MessageFormats::initialize(pybind11::module &m) 
{
    pybind11::module mm = m.def_submodule("MessageFormats");
    mm.attr("__doc__") = "Core message formats used in UMPS.";
    ///------------------------------Base Class------------------------------///
    pybind11::class_<UMPS::Python::MessageFormats::IMessage> iMessage(mm, "IMessage");
    iMessage.def(pybind11::init<> ());
    iMessage.doc() = R""""(
This is a Python wrapper to the IMessage abstract message base class in the
C++ library.  This really should not be used in your Python applications.
For this reason, no functionality has been exposed.
)"""";
    ///-----------------------------Messages---------------------------------///
    pybind11::class_<UMPS::Python::MessageFormats::Messages> messages(mm, "Messages");
    messages.def(pybind11::init<> ());
    messages.doc() = R""""(
This is a container for different types of messages.  This is typically useful
for options classes that require a collection of messages formats to parse on
receival.

Read-Only Properties:
   size : int
      The number of different message types.
   empty : bool
      True indicates there are no message types.
   get: map
      A map containing all the message types.
)"""";
    messages.def("__copy__", [](const Messages &self)
    {
        return Messages(self);
    });
    messages.def("add",
                 &Messages::add,
                 "Adds a message type to the container.");
    messages.def_property_readonly("size",
                                   &Messages::size);
    messages.def_property_readonly("empty",
                                   &Messages::empty);
    messages.def("contains",
                 &Messages::contains,
                 "True indicates the provided message type exists in the container"); 
    messages.def_property_readonly("get",
                                   &Messages::getAll);
    ///----------------------------Failure-----------------------------------///
    pybind11::class_<UMPS::Python::MessageFormats::Failure,
                     UMPS::Python::MessageFormats::IMessage> failure(mm, "Failure");
    failure.def(pybind11::init<> ());
    failure.doc() = R""""(
This is a generic failure message.  This may be returned from a service as a 
catchall.

Properties:
   details : str
      The details of the error message from the service.

Read-Only Properties:
   message_type : str
      The message type.
)"""";
    failure.def("__copy__", [](const Failure &self)
    {
        return Failure(self);
    });
    failure.def_property("details",
                         &Failure::getDetails,
                         &Failure::setDetails);
    failure.def_property_readonly("message_type",
                                  &Failure::getMessageType);
    ///--------------------------------Text----------------------------------///
    pybind11::class_<UMPS::Python::MessageFormats::Text,
                     UMPS::Python::MessageFormats::IMessage> text(mm, "Text");
    text.def(pybind11::init<> ());
    text.doc() = R""""(
This is a container for a text message.  The contents can be arbitrary.
For example, if you need to send the contents of an XML file then you
could use this class.

Properties:
   contents : str
      The contents of the message.

Read-Only Properties:
   message_type : str
      The message type.
)"""";
    text.def("__copy__", [](const Text &self)
    {
        return Text(self);
    });
    text.def_property("contents",
                      &Text::getContents,
                      &Text::setContents);
    text.def_property_readonly("message_type",
                               &Text::getMessageType);
}
