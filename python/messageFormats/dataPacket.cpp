#include "dataPacket.hpp"
#include "urts/messageFormats/dataPacket.hpp"

using namespace PURTS::MessageFormats;

/// C'tor
DataPacket::DataPacket() :
    pImpl(std::make_unique<URTS::MessageFormats::DataPacket<double>> ())
{
}

/// Copy c'tor
DataPacket::DataPacket(const DataPacket &packet)
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
    pImpl = std::make_unique<URTS::MessageFormats::DataPacket<double>>
            (*packet.pImpl);
    return *this;
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
    pImpl->setStartTime(startTime);
}

uint64_t DataPacket::getStartTimeInMicroSeconds() const noexcept
{
    return pImpl->getStartTime();
}

/// End time
uint64_t DataPacket::getEndTimeInMicroSeconds() const
{
    return pImpl->getEndTime();
}

/// Data
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


void PURTS::MessageFormats::initializeDataPacket(pybind11::module &m)
{
    pybind11::class_<PURTS::MessageFormats::DataPacket> o(m, "DataPacket");
    o.def(pybind11::init<> ());
    o.doc() = "This defines a datapacket.\n\nThe following properties are required:\n   network : The station's network name on which the pick was made.\n    station : The name of the station on which the pick was made.\n    channel : The station's channel name on which the pick was made.\n  location_code : The location code of the station on which the pick was made.\n    time : The pick time in UTC seconds since the epoch.\n    sampling_rate : The sampling rate in Hz.\n\nThe following properties are optional:\n    start_time_in_microseconds : The UTC start time in microseconds from the epoch (Jan 1, 1970)\n   data : The time series data comprising this packet.\n";
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
    o.def_property("starttime_in_microseconds",
                   &DataPacket::getStartTimeInMicroSeconds,
                   &DataPacket::setStartTimeInMicroSeconds);
    o.def_property("sampling_rate",
                   &DataPacket::getSamplingRate,
                   &DataPacket::setSamplingRate);
    o.def_property("data",
                   &DataPacket::getData,
                   &DataPacket::setData);

    o.def_property_readonly("endtime_in_microseconds",
                            &DataPacket::getEndTimeInMicroSeconds);
    o.def("clear",
          &DataPacket::clear,
          "Resets the class.");

    o.def("to_json",
          &DataPacket::toJSON,
          "Serializes the class to a JSON object.  The number of spaces controls the formatting",
          pybind11::arg("nSpaces") = 4); 

/*
    // Pickling rules (makes this class copyable)
    o.def(pybind11::pickle(
        [](const DataPacket &p) {
           auto network = p.getNetwork();
           auto station = p.getStation();
           auto channel = p.getChannel();
           auto locationCode = p.getLocationCode();
           auto identifier = p.getIdentifier();
           auto time = p.getTime(); 
           auto phaseHint = p.getPhaseHint();
           auto algorithm = p.getAlgorithm();
           auto polarity = p.getPolarity();
           return pybind11::make_tuple(network, station, channel, locationCode,
                                       identifier, time, phaseHint, algorithm,
                                       polarity);
        },
        [](pybind11::tuple t) {
           if (t.size() != 9)
           {
               throw std::runtime_error("Invalid state");
           }
           auto network = t[0].cast<std::string> (); 
           auto station = t[1].cast<std::string> (); 
           auto channel = t[2].cast<std::string> (); 
           auto locationCode = t[3].cast<std::string> (); 
           auto identifier = t[4].cast<uint64_t> (); 
           auto time = t[5].cast<double> (); 
           auto phaseHint = t[6].cast<std::string> (); 
           auto algorithm = t[7].cast<std::string> (); 
           auto polarity = t[8].cast<Polarity> (); 
           DataPacket p;
           p.setNetwork(network);
           p.setStation(station);
           p.setChannel(channel);
           p.setLocationCode(locationCode); 
           p.setIdentifier(identifier);
           p.setTime(time);
           p.setPhaseHint(phaseHint);
           p.setAlgorithm(algorithm);
           p.setPolarity(polarity);
           return p;
        }
    ));
*/
}
