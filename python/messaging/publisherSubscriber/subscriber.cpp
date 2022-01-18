#include <iostream>
#ifndef NDEBUG
#include <cassert>
#endif
#include "messaging/publisherSubscriber/subscriber.hpp"
#include "messaging/publisherSubscriber/subscriberOptions.hpp"
#include "messageFormats/dataPacket.hpp"
#include "initialize.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/message.hpp"

using namespace PUMPS::Messaging::PublisherSubscriber;

/// C'tor
Subscriber::Subscriber() :
    mSubscriber(
      std::make_unique<UMPS::Messaging::PublisherSubscriber::Subscriber> ())
{
}

/// Destructor
Subscriber::~Subscriber() = default;

/// Initialize class
void Subscriber::initialize(const SubscriberOptions &options)
{
    auto nativeOptions = options.getNativeClass();
    auto messageTypes = nativeOptions.getMessageTypes();
    mMessageTypes
        = std::make_unique<UMPS::MessageFormats::Messages> (messageTypes);
    mSubscriber->initialize(nativeOptions);
    //mPythonMessageTypes = options.getPythonMessageTypes();
}

bool Subscriber::isInitialized() const noexcept
{
    return mSubscriber->isInitialized();
}

/*
void Subscriber::addSubscription(const PUMPS::MessageFormats::IMessage &message)
{
    auto baseClass = message.getBaseClass();
    std::cout << baseClass->getMessageType() << std::endl;
}
*/

/*
/// Security level
UMPS::Authentication::SecurityLevel
    Subscriber::getSecurityLevel() const noexcept
{
    return mSubscriber->getSecurityLevel();
}

/// Connection endpoint
std::string Subscriber::getEndPoint() const
{
    return mSubscriber->getEndPoint();
}
*/

/// 
std::unique_ptr<PUMPS::MessageFormats::IMessage> Subscriber::receive() const
{
    auto message = mSubscriber->receive();
    auto messageType = message->getMessageType();
    auto outMessageType = mMessageTypes->get(messageType);
#ifndef NDEBUG
    //assert(mPythonMessageTypes.contains(messageType));
#endif
    //auto pythonMessage = mPythonMessageTypes.find(messageType)->second;
/*
try
{
    auto result = pythonMessage->clone(message);
return result;
}
catch (const std::exception &e)
{
 std::cerr << e.what() << std::endl;
}
*/
    auto dataPacket = std::make_unique<MessageFormats::DataPacket> ();
    return dataPacket->clone(message);
/*
    return outMessage;
   //outMessage->fromBaseClass(*message);
//    outMessageType->fromBaseClass(*message);
    auto result = std::make_unique<UMPS::MessageFormats::IMessage> ();
std::cout << "Hey" << std::endl;
    //result->fromBaseClass(*message);
std::cout << "okay" << std::endl;
    return result;
*/
}

void PUMPS::Messaging::PublisherSubscriber::initializeSubscriber(pybind11::module &m)
{
    pybind11::class_<PUMPS::Messaging::PublisherSubscriber::Subscriber>
        o(m, "Subscriber");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This defines a subscriber in the publisher/subscriber communication pattern.  The subscriber connects to the address at which a corresponding publisher is sending messages.

Properties :
   is_initialized : True indicates the subscriber is initialized and ready to receive messages.
   end_point : The IP address to which this socket is connected.
   security_level : The security level used by this connection.
)"""";

    o.def("initialize",
          &Subscriber::initialize,
          "Initializes the subscriber.");

    o.def("receive",
          &Subscriber::receive,
          "Receives a message.");

    o.def_property_readonly("is_initialized",
                            &Subscriber::isInitialized);
                            
    //o.def_property_readonly("end_point",
    //                        &Subscriber::getEndPoint);
 
    //o.def_property_readonly("security_level",
    //                        &Subscriber::getSecurityLevel);
}

