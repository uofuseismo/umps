#include "messageFormats/pick.hpp"
#include "messageFormats/dataPacket.hpp"
#include "messaging/publisherSubscriber/publisher.hpp"
#include "messaging/publisherSubscriber/subscriber.hpp"
#include "umps/version.hpp"
#include <pybind11/pybind11.h>

PYBIND11_MODULE(pyumps, m)
{
    m.attr("__version__") = UMPS_VERSION;
    m.attr("__name__") = "pyumps";
    m.attr("__doc__") = "A Python interface to the Univeristy of Utah Seismgoraph Stations Message Passing System (UMPS).";

    // Messaging
    pybind11::module messageFormatsModule = m.def_submodule("MessageFormats");
    messageFormatsModule.attr("__doc__") = "An assortment of message formats in UMPS.";
    PUMPS::MessageFormats::initializeIMessage(messageFormatsModule);
    PUMPS::MessageFormats::initializePick(messageFormatsModule);
    PUMPS::MessageFormats::initializeDataPacket(messageFormatsModule);

    pybind11::module messagingModule = m.def_submodule("Messaging");
    messagingModule.attr("__doc__") = "Message passing patterns used in UMPS.";
    
    pybind11::module pubsubModule = messagingModule.def_submodule("PublisherSubscriber");
    PUMPS::Messaging::PublisherSubscriber::initializeSubscriber(pubsubModule);

 
}


