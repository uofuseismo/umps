#include <string>
#include <vector>
#include "urts/messageFormats/pick.hpp"
#include "pick.hpp"

using namespace PURTS::MessageFormats;

/// C'tor
Pick::Pick() :
    pImpl(std::make_unique<URTS::MessageFormats::Pick> ())
{
}

/// Destructor
Pick::~Pick() = default;

/// Network
std::string Pick::getNetwork() const
{
    return pImpl->getNetwork();
}

void Pick::setNetwork(const std::string &network)
{
    pImpl->setNetwork(network);
}

/// Station
std::string Pick::getStation() const
{
    return pImpl->getStation();
}

void Pick::setStation(const std::string &station)
{
    pImpl->setStation(station);
}

/// Channel
std::string Pick::getChannel() const
{
    return pImpl->getChannel();
}

void Pick::setChannel(const std::string &channel)
{
    pImpl->setChannel(channel);
}

/// Location code
std::string Pick::getLocationCode() const
{
    return pImpl->getLocationCode();
}

void Pick::setLocationCode(const std::string &location)
{
    pImpl->setLocationCode(location);
}

/// Time
double Pick::getTime() const
{
    return pImpl->getTime();
}

void Pick::setTime(const double time) noexcept
{
    pImpl->setTime(time);
}

/// Identifier
uint64_t Pick::getIdentifier() const
{
    return pImpl->getIdentifier();
}

void Pick::setIdentifier(const uint64_t id) noexcept
{
    pImpl->setIdentifier(id);
}

void PURTS::MessageFormats::initializePick(pybind11::module &m)
{
    pybind11::class_<PURTS::MessageFormats::Pick> o(m, "Pick");
    o.def(pybind11::init<> ());
    o.doc() = "This defines a pick.\n\nThe following properties are required:\n   network : The station's network name on which the pick was made.\n    station : The name of the station on which the pick was made.\n    channel : The station's channel name on which the pick was made.\n  location_code : The location code of the station on which the pick was made.\n    time : The pick time in UTC seconds since the epoch.\n    identifier : The unique pick identifier\n\nThe following properties are optional:\n    phase_hint : The seismic phase arrival's hint - e.g., P or S.\n    algorithm : The algorithm that made the pick\n";
    o.def_property("network",
                   &Pick::getNetwork,
                   &Pick::setNetwork);
    o.def_property("station",
                   &Pick::getStation,
                   &Pick::setStation);
    o.def_property("channel",
                   &Pick::getChannel,
                   &Pick::setChannel);
    o.def_property("location_code",
                   &Pick::getLocationCode,
                   &Pick::setLocationCode);

    o.def_property("time",
                   &Pick::getTime,
                   &Pick::setTime);

/*
    o.def_property("tolerance",
*/

}
