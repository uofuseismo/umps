#include "messageFormats/pick.hpp"
#include "messageFormats/dataPacket.hpp"
//#include "messaging/publisherSubscriber/publisher.hpp"
//#include "messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "initialize.hpp"
#include "proxyBroadcasts.hpp"
#include "services.hpp"
#include "log.hpp"
#include "messaging.hpp"
#include "umps/authentication/enums.hpp"
#include "umps/version.hpp"
#include "python/authentication.hpp"
#include <pybind11/pybind11.h>

PYBIND11_MODULE(umpspy, m)
{
    m.attr("__version__") = UMPS_VERSION;
    m.attr("__name__") = "umpspy";
    m.attr("__doc__") = "A Python interface to the Univeristy of Utah Seismograph Stations Message Passing System (UMPS).";
/*
    // Enums
    pybind11::enum_<UMPS::Authentication::SecurityLevel> (m, "SecurityLevel")
        .value("Grasslands", UMPS::Authentication::SecurityLevel::Grasslands,
               "All authentications are accepted.")
        .value("Strawhouse", UMPS::Authentication::SecurityLevel::Strawhouse,
               "IP addresses may be validated.")
        .value("Woodhouse",  UMPS::Authentication::SecurityLevel::Woodhouse,
               "A user name and password must be provided and IP addresses may be validated.")
        .value("Stonehouse",  UMPS::Authentication::SecurityLevel::Stonehouse,
               "A key exchange will be performed and IP addresses may be validated.");
*/

    // Logging
    PUMPS::Logging::initializeLogging(m);
 
    // Message formats
    pybind11::module messageFormatsModule = m.def_submodule("MessageFormats");
    messageFormatsModule.attr("__doc__") = "An assortment of message formats in UMPS.";
    PUMPS::MessageFormats::initializeIMessage(messageFormatsModule);
    PUMPS::MessageFormats::initializeMessages(messageFormatsModule);
    //PUMPS::MessageFormats::initializePick(messageFormatsModule);
    PUMPS::MessageFormats::initializeDataPacket(messageFormatsModule);

    // Messaging
    //pybind11::module messagingModule = m.def_submodule("Messaging");
    //messagingModule.attr("__doc__") = "Message passing patterns used in UMPS.";
    PUMPS::Messaging::initializeMessaging(m);

    // Authentication
    UMPS::Python::Authentication::initialize(m);
/*
    pybind11::module authenticationModule = m.def_submodule("Authentication");
    authenticationModule.attr("__doc__") = "ZeroMQ Authentication Protocol patterns used in UMPS.";
    PUMPS::Authentication::initializeZAPOptions(authenticationModule);
    PUMPS::Authentication::initializeKeys(authenticationModule);
*/
    // Manage specific services
    pybind11::module servicesModule = m.def_submodule("Services");
    servicesModule.attr("__doc__") = "An assortment of services.";
    PUMPS::Services::initializeCommand(servicesModule);

    // Message-specific broadcasts
    pybind11::module broadcastsModule = m.def_submodule("ProxyBroadcasts");
    broadcastsModule.attr("__doc__") = "An assortment of modules for publishing to and receiving messages with pre-defined message types."; 
    PUMPS::ProxyBroadcasts::initializeDataPacketSubscriber(broadcastsModule);
    PUMPS::ProxyBroadcasts::initializeDataPacketSubscriberOptions(broadcastsModule);
    PUMPS::ProxyBroadcasts::initializeHeartbeat(broadcastsModule);
    
/*
    pybind11::module pubsubModule = messagingModule.def_submodule("PublisherSubscriber");
    //PUMPS::Messaging::XPublisherXSubscriber::initializePublisherOptions(messagingModule);
    PUMPS::Messaging::PublisherSubscriber::initializeSubscriberOptions(pubsubModule);
    PUMPS::Messaging::PublisherSubscriber::initializeSubscriber(pubsubModule);
*/
}