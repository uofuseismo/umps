#include <chrono>
#include <string>
#include "proxyBroadcasts/dataPacketSubscriberOptions.hpp"
#include "authentication/zapOptions.hpp"
#include "umps/proxyBroadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "initialize.hpp"
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

using namespace PUMPS::ProxyBroadcasts;

/// C'tor
DataPacketSubscriberOptions::DataPacketSubscriberOptions() :
    pImpl(std::make_unique<UMPS::ProxyBroadcasts::DataPacket::SubscriberOptions<double>> ()) 
{
}

/// Copy c'tor
DataPacketSubscriberOptions::DataPacketSubscriberOptions(
    const DataPacketSubscriberOptions &options)
{
    *this = options;
}

/// Move c'tor
DataPacketSubscriberOptions::DataPacketSubscriberOptions(
    DataPacketSubscriberOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Destructor
DataPacketSubscriberOptions::~DataPacketSubscriberOptions() = default;

/// Reset the class
void DataPacketSubscriberOptions::clear() noexcept
{
    pImpl->clear();
}

/// Copy assignment
DataPacketSubscriberOptions&
DataPacketSubscriberOptions::operator=(const DataPacketSubscriberOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<UMPS::ProxyBroadcasts::DataPacket::SubscriberOptions<double>> (*options.pImpl);
    return *this;
}

/// Move assignment
DataPacketSubscriberOptions&
DataPacketSubscriberOptions::operator=(DataPacketSubscriberOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Native class
UMPS::ProxyBroadcasts::DataPacket::SubscriberOptions<double> 
    DataPacketSubscriberOptions::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Address
void DataPacketSubscriberOptions::setAddress(const std::string &address)
{
    pImpl->setAddress(address);
}

std::string DataPacketSubscriberOptions::getAddress() const
{
    return pImpl->getAddress();
}

/// ZAPoptions
void DataPacketSubscriberOptions::setZAPOptions(
    const PUMPS::Authentication::ZAPOptions &options)
{
    auto nativeOptions = options.getNativeClass();
    pImpl->setZAPOptions(nativeOptions);
}

PUMPS::Authentication::ZAPOptions DataPacketSubscriberOptions::getZAPOptions() const noexcept
{
    PUMPS::Authentication::ZAPOptions result(pImpl->getZAPOptions());
    return result;
}

/// HWM
void DataPacketSubscriberOptions::setHighWaterMark(const int hwm)
{
    pImpl->setHighWaterMark(hwm);
}

int DataPacketSubscriberOptions::getHighWaterMark() const noexcept
{
    return pImpl->getHighWaterMark();
}

/// Timeout
void DataPacketSubscriberOptions::setTimeOut(const int timeOutIn) noexcept
{
    std::chrono::milliseconds timeOut{timeOutIn};
    pImpl->setTimeOut(timeOut);
}

int DataPacketSubscriberOptions::getTimeOut() const noexcept
{
    return static_cast<int> (pImpl->getTimeOut().count());
}

/// Initialize the class
void PUMPS::ProxyBroadcasts::initializeDataPacketSubscriberOptions(pybind11::module &m) 
{
    pybind11::class_<PUMPS::ProxyBroadcasts::DataPacketSubscriberOptions>
        o(m, "DataPacketSubscriberOptions");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This is a specialized class for setting the options to subscribing to data packet feeds.

Properties:
   address : This is the endpoint from which we will receive data packets.
             This must be set to connect.
   zap_options : These are the ZeroMQ Authentication Protocol options.
                 This must be set in security patterns more stringent than
                 Strawhouse.
   high_water_mark : The high water mark in receiving messages.  Effectively,
                     this is the number of messages that can be cached
                     on this computer's receiving socket.  0 is `infinite'.
   time_out : The time out in milliseconds after which the receiver will
              return.  In this case, the resulting message will have no
              information.  If this is negative then the receiver will
              block forever.  This may be dangerous for your application.

)"""";

   o.def_property("address",
                  &DataPacketSubscriberOptions::getAddress,
                  &DataPacketSubscriberOptions::setAddress);
   o.def_property("zap_options",
                  &DataPacketSubscriberOptions::getZAPOptions,
                  &DataPacketSubscriberOptions::setZAPOptions);
   o.def_property("time_out",
                  &DataPacketSubscriberOptions::getTimeOut,
                  &DataPacketSubscriberOptions::setTimeOut);
   o.def_property("high_water_mark",
                  &DataPacketSubscriberOptions::getHighWaterMark,
                  &DataPacketSubscriberOptions::setHighWaterMark);

   o.def("clear",
         &DataPacketSubscriberOptions::clear,
         "Resets the class.");
}
