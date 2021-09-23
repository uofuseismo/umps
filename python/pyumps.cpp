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
    m.attr("__doc__") = "A Python interface to the UUSS Message Passing System.";

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


