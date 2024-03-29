#include <iostream>
#include <string>
#include <sstream>
#include "messageFormats/dataPacket.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace PUMPS::MessageFormats;

/// C'tor
DataPacket::DataPacket() :
    pImpl(std::make_unique<UMPS::MessageFormats::DataPacket<double>> ())
{
}

/// Copy c'tor
DataPacket::DataPacket(const DataPacket &packet)
{
    *this = packet;
}

DataPacket::DataPacket(
    const UMPS::MessageFormats::DataPacket<double> &packet)
{
    *this = packet;
}

/// Move c'tor
DataPacket::DataPacket(DataPacket &&packet) noexcept
{
    *this = std::move(packet);
}

/// Destructor
DataPacket::~DataPacket() = default;

/// Copy assignment
DataPacket& DataPacket::operator=(const DataPacket &packet)
{
    if (&packet == this){return *this;}
    pImpl = std::make_unique<UMPS::MessageFormats::DataPacket<double>>
            (*packet.pImpl);
    return *this;
}

DataPacket& DataPacket::operator=(
    const UMPS::MessageFormats::DataPacket<double> &packet)
{
    pImpl = std::make_unique<UMPS::MessageFormats::DataPacket<double>> (packet);
    return *this;
}

UMPS::MessageFormats::DataPacket<double>
    DataPacket::getNativeClass() const noexcept
{
    return *pImpl;
}

/// Move assignment
DataPacket& DataPacket::operator=(DataPacket &&packet) noexcept
{
    if (&packet == this){return *this;}
    pImpl = std::move(packet.pImpl);
    return *this;
}

/// Network
std::string DataPacket::getNetwork() const
{
    return pImpl->getNetwork();
}

void DataPacket::setNetwork(const std::string &network)
{
    pImpl->setNetwork(network);
}

/// Station
std::string DataPacket::getStation() const
{
    return pImpl->getStation();
}

void DataPacket::setStation(const std::string &station)
{
    pImpl->setStation(station);
}

/// Channel
std::string DataPacket::getChannel() const
{
    return pImpl->getChannel();
}

void DataPacket::setChannel(const std::string &channel)
{
    pImpl->setChannel(channel);
}

/// Location code
std::string DataPacket::getLocationCode() const
{
    return pImpl->getLocationCode();
}

void DataPacket::setLocationCode(const std::string &location)
{
    pImpl->setLocationCode(location);
}

/// Reset class
void DataPacket::clear() noexcept
{
    pImpl->clear();
}

/// Sampling rate
void DataPacket::setSamplingRate(const double df)
{
    pImpl->setSamplingRate(df);
}

double DataPacket::getSamplingRate() const
{
    return pImpl->getSamplingRate();
}

/// Start time
void DataPacket::setStartTimeInMicroSeconds(const int64_t startTime) noexcept
{
    std::chrono::microseconds startTimeMuS{startTime};
    pImpl->setStartTime(startTimeMuS);
}

int64_t DataPacket::getStartTimeInMicroSeconds() const noexcept
{
    return pImpl->getStartTime().count();
}

/// End time
int64_t DataPacket::getEndTimeInMicroSeconds() const
{
    return pImpl->getEndTime().count();
}

/// Data
void DataPacket::setDataFromVector(const std::vector<double> &x)
{
    pImpl->setData(x);
}

void DataPacket::setData(pybind11::array_t<double, pybind11::array::c_style |
                         pybind11::array::forcecast> &x)
{
    pybind11::buffer_info xbuf = x.request();
    auto len = static_cast<int> (xbuf.size);
    const double *xPtr = (double *) (xbuf.ptr);
    if (xPtr == nullptr)
    {
        throw std::invalid_argument("x is null");
    }
    pImpl->setData(len, xPtr);
}

pybind11::array_t<double> DataPacket::getData() const
{
    auto nSamples = pImpl->getNumberOfSamples();
    auto y = pybind11::array_t<double, pybind11::array::c_style> (nSamples);
    pybind11::buffer_info ybuf = y.request();
    auto yPtr = static_cast<double *> (ybuf.ptr);
    const double *dataPtr = pImpl->getDataPointer();
    std::copy(dataPtr, dataPtr + nSamples, yPtr);
    return y;
}

std::vector<double> DataPacket::getDataAsVector() const
{
    return pImpl->getData();
}

void DataPacket::fromBaseClass(UMPS::MessageFormats::IMessage &message)
{
    if (message.getMessageType() != pImpl->getMessageType())
    {
        throw std::invalid_argument("Expecting message type: " 
                                  + pImpl->getMessageType()
                                  + " but given: "
                                  + message.getMessageType());
    }
    pImpl = static_unique_pointer_cast<UMPS::MessageFormats::DataPacket<double>> (message.clone());
}

std::unique_ptr<IMessage> DataPacket::clone(
    const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const
{
std::cout << "cloning" << std::endl;
   if (message->getMessageType() != pImpl->getMessageType())
   {
       throw std::invalid_argument("Expecting: " + pImpl->getMessageType()
                                 + " but got: " + message->getMessageType());
   }
   auto copy = static_unique_pointer_cast<UMPS::MessageFormats::DataPacket<double>>
               (message->clone());
   auto result = std::make_unique<DataPacket> (*copy);
   return result;
}

std::unique_ptr<IMessage> DataPacket::createInstance() const
{
   auto result = std::make_unique<DataPacket> ();
   return result; 
}

/// JSON
std::string DataPacket::toJSON(const int nSpaces) const
{
    return pImpl->toJSON(nSpaces);
} 

/// Message type
std::string DataPacket::getMessageType() const noexcept
{
    return pImpl->getMessageType();
}

/// Base class
std::unique_ptr<UMPS::MessageFormats::IMessage>
    DataPacket::getBaseClass() const noexcept
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> message
        = std::make_unique<UMPS::MessageFormats::DataPacket<double>>
          (getNativeClass());
    return message;
}


void PUMPS::MessageFormats::initializeDataPacket(pybind11::module &m)
{
    pybind11::class_<DataPacket, IMessage> o(m, "DataPacket");
    o.def(pybind11::init<> ());
    o.doc() = R""""(
This defines a datapacket.

Required Properties:

   network : The station's network name on which the pick was made.
   station : The name of the station on which the pick was made.
   channel : The station's channel name on which the pick was made.
   location_code : The location code of the station on which the pick was made.
   time : The pick time in UTC seconds since the epoch.
   sampling_rate : The sampling rate in Hz.

Optional Properties:

   start_time_in_microseconds : The UTC start time in microseconds from the epoch (Jan 1, 1970)
   data : The time series data comprising this packet.
)"""";
    o.def_property("network",
                   &DataPacket::getNetwork,
                   &DataPacket::setNetwork);
    o.def_property("station",
                   &DataPacket::getStation,
                   &DataPacket::setStation);
    o.def_property("channel",
                   &DataPacket::getChannel,
                   &DataPacket::setChannel);
    o.def_property("location_code",
                   &DataPacket::getLocationCode,
                   &DataPacket::setLocationCode);
    o.def_property("start_time_in_microseconds",
                   &DataPacket::getStartTimeInMicroSeconds,
                   &DataPacket::setStartTimeInMicroSeconds);
    o.def_property("sampling_rate",
                   &DataPacket::getSamplingRate,
                   &DataPacket::setSamplingRate);
    o.def_property("data",
                   &DataPacket::getData,
                   &DataPacket::setData);
    o.def_property_readonly("end_time_in_microseconds",
                            &DataPacket::getEndTimeInMicroSeconds);
    o.def("clear",
          &DataPacket::clear,
          "Resets the class.");

    o.def("to_json",
          &DataPacket::toJSON,
          "Serializes the class to a JSON object.  The number of spaces controls the formatting",
          pybind11::arg("nSpaces") = 4); 

    // Pickling rules (makes this class copyable)
    o.def(pybind11::pickle(
        [](const DataPacket &p) {
           auto network = p.getNetwork();
           auto station = p.getStation();
           auto channel = p.getChannel();
           auto locationCode = p.getLocationCode();
           auto samplingRate = p.getSamplingRate();
           auto startTimeMuS = p.getStartTimeInMicroSeconds();
           auto data = p.getDataAsVector();
           auto nSamples = static_cast<int> (data.size());
           std::string x;
           if (nSamples > 0)
           {
               x.reserve(16*data.size());
               for (size_t i = 0; i < data.size(); ++i)
               {
                   x = x + std::to_string(data[i]);
                   if (i < data.size() - 1){x = x + ",";}
               }
           }
           return pybind11::make_tuple(network, station, channel, locationCode,
                                       samplingRate, startTimeMuS, nSamples,
                                       x);
        },
        [](pybind11::tuple t) {
           if (t.size() != 8)
           {
               throw std::runtime_error("Invalid state");
           }
           auto network = t[0].cast<std::string> (); 
           auto station = t[1].cast<std::string> (); 
           auto channel = t[2].cast<std::string> (); 
           auto locationCode = t[3].cast<std::string> (); 
           auto samplingRate = t[4].cast<double> ();
           auto startTimeMuS = t[5].cast<uint64_t> ();
           auto nSamples = t[6].cast<int> ();
           auto cSignal = t[7].cast<std::string> ();
           DataPacket p;
           p.setNetwork(network);
           p.setStation(station);
           p.setChannel(channel);
           p.setLocationCode(locationCode); 
           if (samplingRate > 0){p.setSamplingRate(samplingRate);}
           p.setStartTimeInMicroSeconds(startTimeMuS);
           if (cSignal.size() > 0 && nSamples > 0)
           {
               std::vector<double> signal;
               signal.reserve(nSamples);
               std::stringstream ss(cSignal);
               for (double d; ss >> d;)
               {
                   signal.push_back(d);
                   if (ss.peek() == ','){ss.ignore();}
               }
               p.setDataFromVector(signal);
           }
           return p;
        }
    ));
}
