#include <iostream>
#include "messaging/publisherSubscriber/subscriberOptions.hpp"
#include "authentication/zapOptions.hpp"
#include "messageFormats/message.hpp"
#include "messageFormats/messages.hpp"
#include "initialize.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"

using namespace PUMPS::Messaging::PublisherSubscriber;

/// C'tor
SubscriberOptions::SubscriberOptions() :
    pImpl(std::make_unique<UMPS::Messaging::PublisherSubscriber::SubscriberOptions> ())
{
}

SubscriberOptions::SubscriberOptions(const SubscriberOptions &options)
{
    *this = options;
}

SubscriberOptions::SubscriberOptions(SubscriberOptions &&options) noexcept
{
    *this = std::move(options);
}

SubscriberOptions& SubscriberOptions::operator=(
    const SubscriberOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<UMPS::Messaging::PublisherSubscriber::SubscriberOptions> (*options.pImpl);
/*
    for (const auto &m : options.mPythonMessageTypes)
    {
        mPythonMessageTypes.push_back(m->createInstance());
    } 
*/
    return *this;
}

SubscriberOptions& SubscriberOptions::operator=(
    SubscriberOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    //mPythonMessageTypes = std::move(options.mPythonMessageTypes);
    return *this;
}

UMPS::Messaging::PublisherSubscriber::SubscriberOptions 
    SubscriberOptions::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Get/set address
void SubscriberOptions::setAddress(const std::string &address)
{
    pImpl->setAddress(address);
}

std::string SubscriberOptions::getAddress() const
{
    return pImpl->getAddress();
}

bool SubscriberOptions::haveAddress() const noexcept
{
    return pImpl->haveAddress();
}

/// Message types
void SubscriberOptions::setMessageTypes(
    const PUMPS::MessageFormats::Messages &messages)
{
    auto messageTypes = messages.getNativeClass();
    pImpl->setMessageTypes(messageTypes);
    //mPythonMessageTypes = messages.getPythonMessageTypes();
}

PUMPS::MessageFormats::Messages SubscriberOptions::getMessageTypes() const
{
    auto messageTypes = pImpl->getMessageTypes();
    PUMPS::MessageFormats::Messages result(messageTypes);
    return result;
}

bool SubscriberOptions::haveMessageTypes() const noexcept
{
    return pImpl->haveMessageTypes();
}

/*
std::vector<std::unique_ptr<PUMPS::MessageFormats::IMessage>>
    SubscriberOptions::getPythonMessageTypes() const
{
    return mPythonMessageTypes;
}
*/

/// The high water mark
void SubscriberOptions::setHighWaterMark(const int hwm)
{
    pImpl->setHighWaterMark(hwm);
}

int SubscriberOptions::getHighWaterMark() const noexcept
{
    return pImpl->getHighWaterMark();
}

/// ZAP
void SubscriberOptions::setZAPOptions(
    const PUMPS::Authentication::ZAPOptions &options)
{
    auto nativeOptions = options.getNativeClass();
    pImpl->setZAPOptions(nativeOptions);
}

PUMPS::Authentication::ZAPOptions
    SubscriberOptions::getZAPOptions() const noexcept
{
    PUMPS::Authentication::ZAPOptions result(pImpl->getZAPOptions());
    return result;    
}

/// Destructor
SubscriberOptions::~SubscriberOptions() = default;

void PUMPS::Messaging::PublisherSubscriber::initializeSubscriberOptions(
    pybind11::module &m)
{
    pybind11::class_<PUMPS::Messaging::PublisherSubscriber::SubscriberOptions>
        o(m, "SubscriberOptions");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This defines the subscriber options for use in a publisher/subscriber
communication pattern.

Required Properties :

   address : The address to which to connect.  This must be set.
   message_types: The types of messages that may be received by the subscriber.

Optional Properties :
   high_water_mark : The ZeroMQ socket will buffer this many messages.  After
                     the high-water mark is exceeded messages will be missed by
                     the subscriber.  Note, 0 means ZeroMQ will attempt to
                     buffer an infinite number of messages.
   zap_options : ZeroMQ Authentication Protocol options.
                 If the security level exceeds grasslands then this likely
                 will be necessary to establish a connection.
)"""";

    o.def_property("address",
                   &SubscriberOptions::getAddress,
                   &SubscriberOptions::setAddress);

    o.def_property("message_types",
                   &SubscriberOptions::getMessageTypes,
                   &SubscriberOptions::setMessageTypes);

    o.def_property("high_water_mark",
                   &SubscriberOptions::getHighWaterMark,
                   &SubscriberOptions::setHighWaterMark);

    o.def_property("zap_options",
                   &SubscriberOptions::getZAPOptions,
                   &SubscriberOptions::setZAPOptions);

/*
    o.def("add_subscription",
          &Subscriber::addSubscription,
          "Subscribes to a message of a given type.  The specified type will allow the library to identify the message types that interests the subscriber as well as providing a way to serialize the given message.");
*/

}


