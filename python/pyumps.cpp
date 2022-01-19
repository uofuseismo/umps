#include "messageFormats/pick.hpp"
#include "messageFormats/dataPacket.hpp"
//#include "messaging/publisherSubscriber/publisher.hpp"
//#include "messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "initialize.hpp"
#include "umps/authentication/enums.hpp"
#include "umps/version.hpp"
#include <pybind11/pybind11.h>

PYBIND11_MODULE(pyumps, m)
{
    m.attr("__version__") = UMPS_VERSION;
    m.attr("__name__") = "pyumps";
    m.attr("__doc__") = "A Python interface to the Univeristy of Utah Seismgoraph Stations Message Passing System (UMPS).";
    // Enums
    pybind11::enum_<UMPS::Authentication::SecurityLevel> (m, "SecurityLevel")
        .value("Grasslands", UMPS::Authentication::SecurityLevel::GRASSLANDS,
               "All authentications are accepted.")
        .value("Strawhouse", UMPS::Authentication::SecurityLevel::STRAWHOUSE,
               "IP addresses may be validated.")
        .value("Woodhouse",  UMPS::Authentication::SecurityLevel::WOODHOUSE,
               "A user name and password must be provided and IP addresses may be validated.")
        .value("Stonehouse",  UMPS::Authentication::SecurityLevel::STONEHOUSE,
               "A key exchange will be performed and IP addresses may be validated.");

    // Message formats
    pybind11::module messageFormatsModule = m.def_submodule("MessageFormats");
    messageFormatsModule.attr("__doc__") = "An assortment of message formats in UMPS.";
    PUMPS::MessageFormats::initializeIMessage(messageFormatsModule);
    PUMPS::MessageFormats::initializeMessages(messageFormatsModule);
    PUMPS::MessageFormats::initializePick(messageFormatsModule);
    PUMPS::MessageFormats::initializeDataPacket(messageFormatsModule);

    // Messaging
    pybind11::module messagingModule = m.def_submodule("Messaging");
    messagingModule.attr("__doc__") = "Message passing patterns used in UMPS.";

    // Authentication
    pybind11::module authenticationModule = m.def_submodule("Authentication");
    authenticationModule.attr("__doc__") = "ZeroMQ Authentication Protocol patterns used in UMPS.";
    PUMPS::Authentication::initializeZAPOptions(authenticationModule);
    PUMPS::Authentication::initializeKeys(authenticationModule);

    // Message-specific broadcasts
    pybind11::module broadcastsModule = m.def_submodule("ProxyBroadcasts");
    broadcastsModule.attr("__doc__") = "An assortment of modules for publishing to and receiving messages with pre-defined message types."; 
    PUMPS::ProxyBroadcasts::initializeDataPacketSubscriber(broadcastsModule);
    PUMPS::ProxyBroadcasts::initializeDataPacketSubscriberOptions(broadcastsModule);
    
    pybind11::module pubsubModule = messagingModule.def_submodule("PublisherSubscriber");
    //PUMPS::Messaging::XPublisherXSubscriber::initializePublisherOptions(messagingModule);
    PUMPS::Messaging::PublisherSubscriber::initializeSubscriberOptions(pubsubModule);
    PUMPS::Messaging::PublisherSubscriber::initializeSubscriber(pubsubModule);
}
