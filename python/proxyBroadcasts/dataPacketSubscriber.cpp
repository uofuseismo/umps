#include <iostream>
#include "proxyBroadcasts/dataPacketSubscriber.hpp"
#include "proxyBroadcasts/dataPacketSubscriberOptions.hpp"
#include "messageFormats/dataPacket.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriber.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "initialize.hpp"

using namespace PUMPS::ProxyBroadcasts;

/// C'tor
DataPacketSubscriber::DataPacketSubscriber() :
    pImpl(std::make_unique<UMPS::ProxyBroadcasts::DataPacket::Subscriber<double>> ())
{
}

/// Destructor
DataPacketSubscriber::~DataPacketSubscriber() = default;

/// Initialize
void DataPacketSubscriber::initialize(const DataPacketSubscriberOptions &options)
{
    auto nativeOptions = options.getNativeClass();
    pImpl->initialize(nativeOptions);
}

/// Initialized?
bool DataPacketSubscriber::isInitialized() const noexcept
{
    return pImpl->isInitialized();
}

/// Receive a message
PUMPS::MessageFormats::DataPacket DataPacketSubscriber::receive() const
{
    PUMPS::MessageFormats::DataPacket result;
    auto message = pImpl->receive();
    if (message != nullptr){result.fromBaseClass(*message);} 
    return result;
}


/// Initialize the class
void PUMPS::ProxyBroadcasts::initializeDataPacketSubscriber(pybind11::module &m)
{
    pybind11::class_<PUMPS::ProxyBroadcasts::DataPacketSubscriber>
        o(m, "DataPacketSubscriber");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This is a specialized class for subscribing to data packet feeds.

Read-only Properties :

   is_initialized : True indicates the subscriber is initialized and ready
                    to receive messages.

)"""";

   o.def_property_readonly("is_initialized",
                           &DataPacketSubscriber::isInitialized);
   o.def("receive",
         &DataPacketSubscriber::receive, 
         "Receives a message from the broadcast.  The class must be initialized prior to calling this.  Note, the message may have no information.  In this case, the receive timed out.");
   o.def("initialize",
         &DataPacketSubscriber::initialize,
         "Initializes the subscriber.  Note, the address to which to connect must, at minimum, be set on the options");

}
