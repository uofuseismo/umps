#include <string>
#include <vector>
#include "messageFormats/pick.hpp"
#include "urts/messageFormats/pick.hpp"

using namespace PURTS::MessageFormats;

/// C'tor
Pick::Pick() :
    pImpl(std::make_unique<URTS::MessageFormats::Pick> ())
{
}

/// Copy c'tor
Pick::Pick(const Pick &pick)
{
    *this = pick;
}

/// Move c'tor
Pick::Pick(Pick &&pick) noexcept
{
    *this = std::move(pick);
}

/// Copy assignment
Pick& Pick::operator=(const Pick &pick)
{
    if (&pick == this){return *this;}
    pImpl = std::make_unique<URTS::MessageFormats::Pick> (*pick.pImpl);
    return *this;
}

/// Move assignment
Pick& Pick::operator=(Pick &&pick) noexcept
{
    if (&pick == this){return *this;}
    pImpl = std::move(pick.pImpl);
    return *this;
}

/// Get native class
URTS::MessageFormats::Pick Pick::getNativeClass() const noexcept
{
    return *pImpl;
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

// Algorithm
std::string Pick::getAlgorithm() const noexcept
{
    return pImpl->getAlgorithm();
}

void Pick::setAlgorithm(const std::string &algorithm) noexcept
{
    pImpl->setAlgorithm(algorithm);
}

/// Phase hint
std::string Pick::getPhaseHint() const noexcept
{
    return pImpl->getPhaseHint();
}

void Pick::setPhaseHint(const std::string &hint) noexcept
{
    pImpl->setPhaseHint(hint);
}

/// Reset class
void Pick::clear() noexcept
{
    pImpl->clear();
}

/// Polarity
void Pick::setPolarity(const Polarity polarity) noexcept
{
    auto newPolarity
        = static_cast<URTS::MessageFormats::Pick::Polarity> (polarity);
     pImpl->setPolarity(newPolarity);
}

Polarity Pick::getPolarity() const noexcept
{
    auto polarity = static_cast<Polarity> (pImpl->getPolarity());
    return polarity;
}

/// Message type
std::string Pick::getMessageType() const noexcept
{
    return pImpl->getMessageType();
}

/// JSON
std::string Pick::toJSON(const int nSpaces) const
{
    return pImpl->toJSON(nSpaces);
}

/// Useful for calling messaging functions
std::unique_ptr<URTS::MessageFormats::IMessage>
    Pick::getBaseClass() const noexcept
{
    std::unique_ptr<URTS::MessageFormats::IMessage> message
        = std::make_unique<URTS::MessageFormats::Pick> (getNativeClass()); 
    return message;
}


void PURTS::MessageFormats::initializePick(pybind11::module &m)
{
    pybind11::class_<Pick, IMessage> o(m, "Pick");
    o.def(pybind11::init<> ());
    o.doc() = "This defines a pick.\n\nThe following properties are required:\n   network : The station's network name on which the pick was made.\n    station : The name of the station on which the pick was made.\n    channel : The station's channel name on which the pick was made.\n  location_code : The location code of the station on which the pick was made.\n    time : The pick time in UTC seconds since the epoch.\n    identifier : The unique pick identifier\n\nThe following properties are optional:\n    polarity : The polarity which can be -1 (down), +1 (up), or 0 (unkown).\n    phase_hint : The seismic phase arrival's hint - e.g., P or S.\n    algorithm : The algorithm that made the pick\n";
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

    o.def_property("identifier",
                   &Pick::getIdentifier,
                   &Pick::setIdentifier);

    o.def_property("polarity",
                   &Pick::getPolarity,
                   &Pick::setPolarity);

    o.def_property("phase_hint",
                   &Pick::getPhaseHint,
                   &Pick::setPhaseHint);

    o.def_property("algorithm",
                   &Pick::getAlgorithm,
                   &Pick::setAlgorithm);

    o.def_property_readonly("message_type",
                            &Pick::getMessageType);

    o.def("clear",
          &Pick::clear,
          "Resets the class.");

    o.def("to_json",
          &Pick::toJSON,
          "Serializes the class to a JSON object.  The number of spaces controls the formatting",
          pybind11::arg("nSpaces") = 4);

/*
    m.def("pick_base_class", []()
    {
        std::unique_ptr<IMessage> result = std::make_unique<Pick> ();
        return std::unique_ptr<IMessage> (new Pick);
    });
*/


    pybind11::enum_<PURTS::MessageFormats::Polarity> (m, "Polarity")
        .value("up", PURTS::MessageFormats::Polarity::UP,
               "An upwards (compressional) polarity.")
        .value("down", PURTS::MessageFormats::Polarity::DOWN,
               "A downwards (dilitational) polarity.")
        .value("unkonwn", PURTS::MessageFormats::Polarity::UNKNOWN,
               "An unknown polarity.");


    // Pickling rules (makes this class copyable)
    o.def(pybind11::pickle(
        [](const Pick &p) {
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
           Pick p;
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
}
