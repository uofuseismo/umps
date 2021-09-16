#include "messageFormats/pick.hpp"
#include "messageFormats/dataPacket.hpp"
#include "messaging/publisherSubscriber/publisher.hpp"
#include "messaging/publisherSubscriber/subscriber.hpp"
#include "urts/version.hpp"
#include <pybind11/pybind11.h>

PYBIND11_MODULE(pyurts, m)
{
    m.attr("__version__") = URTS_VERSION;
    m.attr("__name__") = "pyurts";
    m.attr("__doc__") = "A toolkit for real-time seismic monitoring at University of Utah Seismograph Stations.";

    // Messaging
    pybind11::module messageFormatsModule = m.def_submodule("MessageFormats");
    messageFormatsModule.attr("__doc__") = "An assortment of message formats in URTS.";
    PURTS::MessageFormats::initializeIMessage(messageFormatsModule);
    PURTS::MessageFormats::initializePick(messageFormatsModule);
    PURTS::MessageFormats::initializeDataPacket(messageFormatsModule);

    pybind11::module messagingModule = m.def_submodule("Messaging");
    messagingModule.attr("__doc__") = "Message passing patterns used in URTS.";
    
    pybind11::module pubsubModule = messagingModule.def_submodule("PublisherSubscriber");
    PURTS::Messaging::PublisherSubscriber::initializeSubscriber(pubsubModule);

 
}


