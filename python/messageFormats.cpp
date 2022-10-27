#include <umps/messageFormats/message.hpp>
#include <umps/messageFormats/messages.hpp>
#include <umps/messageFormats/text.hpp>
#include <umps/messageFormats/failure.hpp>
#include "python/messageFormats.hpp"
#include <private/staticUniquePointerCast.hpp>

using namespace UMPS::Python::MessageFormats;

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
void Messages::add(const IMessage &message)
{
     auto instance = message.createInstance();
     if (instance == nullptr){throw std::invalid_argument("Instance is NULL");}
     auto nativeClass = message.getInstanceOfBaseClass();
     pImpl->add(nativeClass); 
     //mPythonMessageTypes.push_back(std::move(instance));
}

///--------------------------------------------------------------------------///
///                                 Failure Message                          ///
///--------------------------------------------------------------------------///
/// C'tor
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

UMPS::MessageFormats::Failure Failure::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Things to override
std::unique_ptr<IMessage> Failure::clone(
    const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const
{
   if (message->getMessageType() != pImpl->getMessageType())
   {   
       throw std::invalid_argument("Expecting: " + pImpl->getMessageType()
                                 + " but got: " + message->getMessageType());
   }
   auto copy = static_unique_pointer_cast<UMPS::MessageFormats::Failure>
               (message->clone());
   auto result = std::make_unique<Failure> (*copy);
   return result;
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
    ///----------------------------Failure-----------------------------------///
    pybind11::class_<UMPS::Python::MessageFormats::Failure> failure(mm, "Failure");
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
}
