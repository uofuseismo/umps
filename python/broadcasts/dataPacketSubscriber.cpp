#include "broadcasts/dataPacketSubscriber.hpp"
#include "messageFormats/dataPacket.hpp"
#include "umps/broadcasts/dataPacket/subscriber.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "initialize.hpp"

using namespace PUMPS::Broadcasts;

/// C'tor
DataPacketSubscriber::DataPacketSubscriber() :
    pImpl(std::make_unique<UMPS::Broadcasts::DataPacket::Subscriber<double>> ())
{
}

/// Destructor
DataPacketSubscriber::~DataPacketSubscriber() = default;

/// Initialized?
bool DataPacketSubscriber::isInitialized() const noexcept
{
    return pImpl->isInitialized();
}

/// Receive a message
PUMPS::MessageFormats::DataPacket DataPacketSubscriber::receive() const
{
    auto message = pImpl->receive();
    PUMPS::MessageFormats::DataPacket result(*message);
    return result;
}


/// Initialize the class
void PUMPS::Broadcasts::initializeDataPacketSubscriber(pybind11::module &m)
{
    pybind11::class_<PUMPS::Broadcasts::DataPacketSubscriber>
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
/*
   o.def("set_grasslands_server",
         &ZAPOptions::setGrasslandsServer,
         "Sets this as a grasslands server.  This machine will accept all connections.");
   o.def("set_grasslands_client",
         &ZAPOptions::setGrasslandsClient,
         "Sets this as a grasslands client.  This machine will be able to connect to Grasslands servers.");
*/

}
