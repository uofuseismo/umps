#include <string>
#include <umps/messaging/context.hpp>
#include <umps/messageFormats/messages.hpp>
#include <umps/messaging/publisherSubscriber/publisher.hpp>
#include <umps/messaging/publisherSubscriber/publisherOptions.hpp>
#include <umps/messaging/publisherSubscriber/subscriber.hpp>
#include <umps/messaging/publisherSubscriber/subscriberOptions.hpp>
#include <umps/authentication/zapOptions.hpp>
#include <umps/logging/log.hpp>
#include "python/messaging.hpp"
#include "python/authentication.hpp"
#include "python/messageFormats.hpp"
#include "python/logging.hpp"

namespace
{

}

using namespace UMPS::Python::Messaging;
namespace UMessaging = UMPS::Messaging;

///--------------------------------------------------------------------------///
///                              Context                                     ///
///--------------------------------------------------------------------------///

Context::Context(const int nInputThreads) :
    mContext(std::make_shared<UMessaging::Context> (nInputThreads))
{
}

Context::~Context() = default;

std::shared_ptr<UMessaging::Context> Context::getSharedPointer() const
{
    return mContext;
}

///--------------------------------------------------------------------------///
///                        Pub/Sub Subscriber Options                        ///
///--------------------------------------------------------------------------///
/// Constructor
PublisherSubscriber::SubscriberOptions::SubscriberOptions() :
    pImpl(std::make_unique<UMessaging::PublisherSubscriber::SubscriberOptions> ())
{
} 

/// Copy constructor
PublisherSubscriber::SubscriberOptions::SubscriberOptions(
    const PublisherSubscriber::SubscriberOptions &options)
{
    *this = options;
}

/// Copy constructor from UMPS class
PublisherSubscriber::SubscriberOptions::SubscriberOptions(
    const UMessaging::PublisherSubscriber::SubscriberOptions &options)
{
    *this = options;
}

/// Move constructor
PublisherSubscriber::SubscriberOptions::SubscriberOptions(
    PublisherSubscriber::SubscriberOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
PublisherSubscriber::SubscriberOptions&
PublisherSubscriber::SubscriberOptions::operator=(
    const PublisherSubscriber::SubscriberOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<UMessaging::PublisherSubscriber::SubscriberOptions> (*options.pImpl);
    return *this;
}

/// Copy assignment from UMPS class
PublisherSubscriber::SubscriberOptions&
PublisherSubscriber::SubscriberOptions::operator=(
    const UMessaging::PublisherSubscriber::SubscriberOptions &options)
{
    pImpl = std::make_unique<UMessaging::PublisherSubscriber::SubscriberOptions> (options);
    return *this;
}

/// Move assignment
PublisherSubscriber::SubscriberOptions&
PublisherSubscriber::SubscriberOptions::operator=(
    PublisherSubscriber::SubscriberOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Address
void PublisherSubscriber::SubscriberOptions::setAddress(const std::string &address)
{
    pImpl->setAddress(address);
}

std::string PublisherSubscriber::SubscriberOptions::getAddress() const
{
    if (!pImpl->haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->getAddress();
}

/// Messaging
void PublisherSubscriber::SubscriberOptions::setMessageTypes(
    const UMPS::Python::MessageFormats::Messages &messageTypes)
{
    pImpl->setMessageTypes(messageTypes.getNativeClassReference());
}

UMPS::Python::MessageFormats::Messages
PublisherSubscriber::SubscriberOptions::getMessageTypes() const
{
    if (!pImpl->haveMessageTypes())
    {
        throw std::runtime_error("No message types set");
    }
    auto messageTypes = pImpl->getMessageTypes();
    UMPS::Python::MessageFormats::Messages result(messageTypes);
    return result;
}

/// ZAP options
void PublisherSubscriber::SubscriberOptions::setZAPOptions(
    const UMPS::Python::Authentication::ZAPOptions &options)
{
    pImpl->setZAPOptions(options.getNativeClassReference());
}

UMPS::Python::Authentication::ZAPOptions
PublisherSubscriber::SubscriberOptions::getZAPOptions() const noexcept
{
    UMPS::Python::Authentication::ZAPOptions options(pImpl->getZAPOptions());
    return options;
}

/// Time out
void PublisherSubscriber::SubscriberOptions::setReceiveTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->setReceiveTimeOut(timeOut);
}

std::chrono::milliseconds
PublisherSubscriber::SubscriberOptions::getReceiveTimeOut() const noexcept
{
    return pImpl->getReceiveTimeOut();
}

/// HWM
void PublisherSubscriber::SubscriberOptions::setReceiveHighWaterMark(const int hwm)
{
    pImpl->setReceiveHighWaterMark(hwm);
}

int PublisherSubscriber::SubscriberOptions::getReceiveHighWaterMark() const noexcept
{
    return pImpl->getReceiveHighWaterMark();
}

/// Reset class
void PublisherSubscriber::SubscriberOptions::clear() noexcept
{
    pImpl->clear();
}

/// Destructor
PublisherSubscriber::SubscriberOptions::~SubscriberOptions() = default;

///--------------------------------------------------------------------------///
///                        Pub/Sub Publisher Options                         ///
///--------------------------------------------------------------------------///
/// Constructor
PublisherSubscriber::PublisherOptions::PublisherOptions() :
    pImpl(std::make_unique<UMessaging::PublisherSubscriber::PublisherOptions> ())
{
}

/// Copy constructor
PublisherSubscriber::PublisherOptions::PublisherOptions(
    const PublisherSubscriber::PublisherOptions &options)
{
    *this = options;
}

/// Copy constructor from UMPS class
PublisherSubscriber::PublisherOptions::PublisherOptions(
    const UMessaging::PublisherSubscriber::PublisherOptions &options)
{
    *this = options;
}

/// Move constructor
PublisherSubscriber::PublisherOptions::PublisherOptions(
    PublisherSubscriber::PublisherOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
PublisherSubscriber::PublisherOptions&
PublisherSubscriber::PublisherOptions::operator=(
    const PublisherSubscriber::PublisherOptions &options)
{   
    if (&options == this){return *this;}
    pImpl = std::make_unique<UMessaging::PublisherSubscriber::PublisherOptions> (*options.pImpl);
    return *this;
}

/// Copy assignment from UMPS class
PublisherSubscriber::PublisherOptions&
PublisherSubscriber::PublisherOptions::operator=(
    const UMessaging::PublisherSubscriber::PublisherOptions &options)
{
    pImpl = std::make_unique<UMessaging::PublisherSubscriber::PublisherOptions> (options);
    return *this;
}

/// Move assignment
PublisherSubscriber::PublisherOptions&
PublisherSubscriber::PublisherOptions::operator=(
    PublisherSubscriber::PublisherOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Address
void PublisherSubscriber::PublisherOptions::setAddress(const std::string &address)
{
    pImpl->setAddress(address);
}

std::string PublisherSubscriber::PublisherOptions::getAddress() const
{
    if (!pImpl->haveAddress()){throw std::runtime_error("Address not set");}
    return pImpl->getAddress();
}

/// Time out
void PublisherSubscriber::PublisherOptions::setSendTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->setSendTimeOut(timeOut);
}

std::chrono::milliseconds
PublisherSubscriber::PublisherOptions::getSendTimeOut() const noexcept
{
    return pImpl->getSendTimeOut();
}

/// HWM
void PublisherSubscriber::PublisherOptions::setSendHighWaterMark(const int hwm)
{
    pImpl->setSendHighWaterMark(hwm);
}

int PublisherSubscriber::PublisherOptions::getSendHighWaterMark() const noexcept
{
    return pImpl->getSendHighWaterMark();
}

/// ZAP options
void PublisherSubscriber::PublisherOptions::setZAPOptions(
    const UMPS::Python::Authentication::ZAPOptions &options)
{
    pImpl->setZAPOptions(options.getNativeClassReference());
}

UMPS::Python::Authentication::ZAPOptions
PublisherSubscriber::PublisherOptions::getZAPOptions() const noexcept
{
    UMPS::Python::Authentication::ZAPOptions options(pImpl->getZAPOptions());
    return options;
}

/// Reset class
void PublisherSubscriber::PublisherOptions::clear() noexcept
{
    pImpl->clear();
}

/// Destructor
PublisherSubscriber::PublisherOptions::~PublisherOptions() = default;

///--------------------------------------------------------------------------///
///                            Pub/Sub Subscriber                            ///
///--------------------------------------------------------------------------///

PublisherSubscriber::Publisher::Publisher() :
    pImpl(std::make_unique<UMPS::Messaging::PublisherSubscriber::Publisher> ())
{
}

PublisherSubscriber::Publisher::Publisher(
    UMPS::Python::Logging::ILog &logger)
{
    auto nativeLogger = logger.getSharedPointer();
    pImpl = std::make_unique<UMPS::Messaging::PublisherSubscriber::Publisher>
            (nativeLogger);
}

PublisherSubscriber::Publisher::Publisher(
    UMPS::Python::Messaging::Context &context)
{
    auto nativeContext = context.getSharedPointer();
    pImpl = std::make_unique<UMPS::Messaging::PublisherSubscriber::Publisher>
            (nativeContext); 
}


PublisherSubscriber::Publisher::Publisher(
    UMPS::Python::Messaging::Context &context,
    UMPS::Python::Logging::ILog &logger)
{
    auto nativeContext = context.getSharedPointer();
    auto nativeLogger = logger.getSharedPointer();
    pImpl = std::make_unique<UMPS::Messaging::PublisherSubscriber::Publisher>
            (nativeContext, nativeLogger); 
}

/// Destructor
PublisherSubscriber::Publisher::~Publisher() = default;

///--------------------------------------------------------------------------///
///                             Router/Dealer                                ///
///--------------------------------------------------------------------------///

///--------------------------------------------------------------------------///
///                            Initialization                                ///
///--------------------------------------------------------------------------///
void UMPS::Python::Messaging::initialize(pybind11::module &m)
{
    pybind11::module messagingModule = m.def_submodule("Messaging");
    messagingModule.attr("__doc__") = "Message passing patterns used in UMPS.";
    pybind11::class_<UMPS::Python::Messaging::Context,
                     std::shared_ptr<UMPS::Python::Messaging::Context>>
                    context(messagingModule, "Context");
    context.def(pybind11::init<int> ());
    context.doc() = R""""(
This is an UMPS context.  Loosely speaking, a context is like a thread
that is specific to ZeroMQ and is responsible for sending/receiving
messages.  Typically, you'll want to have one context per each 1 Gb/s
of messaging.  Note, for inproc communication the number of threads can
be 0.
)"""";
    ///----------------------------------------------------------------------///
    ///                             Pub/Sub                                  ///
    ///----------------------------------------------------------------------///
    pybind11::module pubSubModule = messagingModule.def_submodule("PublisherSubscriber");
    pubSubModule.attr("__doc__") = "A publisher/subscriber communication pattern.  This is useful for pedagogical reasons only.";
    ///--------------------------Subscriber Options--------------------------///
    pybind11::class_<UMPS::Python::Messaging::PublisherSubscriber::SubscriberOptions>
        pubSubSubscriberOptions(pubSubModule, "SubscriberOptions");
    pubSubSubscriberOptions.def(pybind11::init<> ());
    pubSubSubscriberOptions.doc() = R""""(
The options for a subscriber in a publisher/subscriber pattern.

Required Properties :
    address : str
        The address to which to connect; e.g., tcp://127.0.0.1:5555
    message_types : UMPS::Python::MessageFormats::Messages
        A container of messages types to which to subscribe to.

Optional Properties :
    zap_options : UMPS::Python::Authentication::ZAPOptions
        The ZeroMQ authenticaiton protocol options for authentication this connection.
    receive_time_out : int
        The number of milliseconds to wait when receiving a message.  This is
        typically useful when the thread has other things to do.  A negative
        number will cause this to block indefinitely.
    receive_high_water_mark : int
        Influences the number of messages to leave on the socket.  Zero will 
        be "infinite".  This cannot be negative.
)"""";
    pubSubSubscriberOptions.def("__copy__", [](const PublisherSubscriber::SubscriberOptions &self)
    {
        return PublisherSubscriber::SubscriberOptions(self);
    });
    pubSubSubscriberOptions.def_property("address",
                                         &PublisherSubscriber::SubscriberOptions::getAddress,
                                         &PublisherSubscriber::SubscriberOptions::setAddress);
    pubSubSubscriberOptions.def_property("message_types",
                                         &PublisherSubscriber::SubscriberOptions::getMessageTypes,
                                         &PublisherSubscriber::SubscriberOptions::setMessageTypes); 
    pubSubSubscriberOptions.def_property("zap_options",
                                         &PublisherSubscriber::SubscriberOptions::getZAPOptions,
                                         &PublisherSubscriber::SubscriberOptions::setZAPOptions);
    pubSubSubscriberOptions.def_property("send_time_out",
                                         &PublisherSubscriber::SubscriberOptions::getReceiveTimeOut,
                                         &PublisherSubscriber::SubscriberOptions::setReceiveTimeOut);
    pubSubSubscriberOptions.def_property("send_high_water_mark",
                                         &PublisherSubscriber::SubscriberOptions::getReceiveHighWaterMark,
                                         &PublisherSubscriber::SubscriberOptions::setReceiveHighWaterMark);
    pubSubSubscriberOptions.def("clear",
                                &PublisherSubscriber::SubscriberOptions::clear,
                                "Resets the class.");
    ///-----------------------------Publisher Options------------------------///
    pybind11::class_<UMPS::Python::Messaging::PublisherSubscriber::PublisherOptions>
        pubSubPublisherOptions(pubSubModule, "PublisherOptions");
    pubSubPublisherOptions.def(pybind11::init<> ());
    pubSubPublisherOptions.doc() = R""""(
The options for a publisher in a publisher/subscriber pattern.

Required Properties :
    address : str
        The address to which to connect; e.g., tcp://127.0.0.1:5555

Optional Properties :
    zap_options : UMPS::Python::Authentication::ZAPOptions
        The ZeroMQ authenticaiton protocol options for authentication this connection.
    send_time_out : int
        The number of milliseconds to wait when sending a message.  This will
        likely be zero indicating the process should return immediately.
        A negative number will cause this to block indefinitely.
    send_high_water_mark : int
        Influences the number of messages to leave on the socket.  Zero will
        be "infinite".  This cannot be negative.
)"""";
    pubSubPublisherOptions.def("__copy__", [](const PublisherSubscriber::PublisherOptions &self)
    {
        return PublisherSubscriber::PublisherOptions(self);
    });
    pubSubPublisherOptions.def_property("address",
                                        &PublisherSubscriber::PublisherOptions::getAddress,
                                        &PublisherSubscriber::PublisherOptions::setAddress);
    pubSubPublisherOptions.def_property("zap_options",
                                        &PublisherSubscriber::PublisherOptions::getZAPOptions,
                                        &PublisherSubscriber::PublisherOptions::setZAPOptions);
    pubSubPublisherOptions.def_property("send_time_out",
                                        &PublisherSubscriber::PublisherOptions::getSendTimeOut,
                                        &PublisherSubscriber::PublisherOptions::setSendTimeOut);
    pubSubPublisherOptions.def_property("send_high_water_mark",
                                        &PublisherSubscriber::PublisherOptions::getSendHighWaterMark,
                                        &PublisherSubscriber::PublisherOptions::setSendHighWaterMark);
    pubSubPublisherOptions.def("clear",
                               &PublisherSubscriber::PublisherOptions::clear,
                               "Resets the class.");
    ///---------------------------Publisher----------------------------------///
    pybind11::class_<UMPS::Python::Messaging::PublisherSubscriber::Publisher>
        pubSubPublisher(pubSubModule, "PublisherOptions");
    pubSubPublisher.def(pybind11::init<> ());
    pubSubPublisher.def(pybind11::init<UMPS::Python::Messaging::Context &,
                                       UMPS::Python::Logging::ILog &> ());
    pubSubPublisher.doc() = R""""(
The publisher in a publisher/subscriber pattern.
)"""";
 
}
