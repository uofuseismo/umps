#include <iostream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "urts/messageFormats/pick.hpp"
#include "private/isEmpty.hpp"

#define MESSAGE_TYPE "Pick"

using namespace URTS::MessageFormats;

namespace
{

nlohmann::json toJSONObject(const Pick &pick)
{
    nlohmann::json obj;
    // Essential stuff (this will throw): 
    // Network/Station/Channel/Location
    obj["MessageType"] = pick.getMessageType();
    obj["Network"] = pick.getNetwork();
    obj["Station"] = pick.getStation();
    obj["Channel"] = pick.getChannel();
    obj["LocationCode"] = pick.getLocationCode();
    // Pick time
    obj["Time"] = pick.getTime();
    // Identifier
    obj["Identifier"] = pick.getIdentifier();
    // Non-essential stuff:
    auto phaseHint = pick.getPhaseHint();
    if (!phaseHint.empty())
    {
        obj["PhaseHint"] = phaseHint;
    }
    else
    {
        obj["PhaseHint"] = nullptr;
    }
    // Polarity
    obj["Polarity"] = static_cast<int> (pick.getPolarity()); 
    // Algorithm
    obj["Algorithm"] = pick.getAlgorithm();
    return obj;
}

Pick objectToPick(const nlohmann::json obj)
{
    Pick pick;
    if (obj["MessageType"] != pick.getMessageType())
    {
        throw std::invalid_argument("Message has invalid message type");
    }
    // Essential stuff
    pick.setNetwork(obj["Network"].get<std::string> ());
    pick.setStation(obj["Station"].get<std::string> ());
    pick.setChannel(obj["Channel"].get<std::string> ());
    pick.setLocationCode(obj["LocationCode"].get<std::string> ());
    pick.setTime(obj["Time"].get<double> ());
    pick.setIdentifier(obj["Identifier"].get<uint64_t> ());
    // Optional stuff
    pick.setPolarity(
        static_cast<Pick::Polarity> (obj["Polarity"].get<int> ()));
    if (!obj["PhaseHint"].is_null())
    {
        pick.setPhaseHint(obj["PhaseHint"].get<std::string> ());
    }
    if (!obj["Algorithm"].is_null())
    {
        pick.setAlgorithm(obj["Algorithm"].get<std::string> ());
    }
    return pick;
}

Pick fromJSONMessage(const std::string &message)
{
    auto obj = nlohmann::json::parse(message);
    return objectToPick(obj);
}

Pick fromCBORMessage(const uint8_t *message, const size_t length)
{
    auto obj = nlohmann::json::from_cbor(message, message + length);
    return objectToPick(obj);
}


}

class Pick::PickImpl
{
public:
    std::string mNetwork;
    std::string mStation;
    std::string mChannel;
    std::string mLocationCode;
    std::string mPhaseHint;
    std::string mAlgorithm = "unspecified";
    double mTime = 0;
    uint64_t mIdentifier = 0;
    Pick::Polarity mPolarity = Pick::Polarity::UNKNOWN;
    bool mHaveTime = false;
    bool mHaveIdentifier = false;
};

/// C'tor
Pick::Pick() :
    pImpl(std::make_unique<PickImpl> ())
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
    pImpl = std::make_unique<PickImpl> (*pick.pImpl);
    return *this;
}

/// Move assignment
Pick& Pick::operator=(Pick &&pick) noexcept
{
    if (&pick == this){return *this;}
    pImpl = std::move(pick.pImpl);
    return *this;
}

/// Destructor
Pick::~Pick() = default;

/// Reset the class 
void Pick::clear() noexcept
{
    pImpl->mNetwork.clear();
    pImpl->mStation.clear();
    pImpl->mChannel.clear();
    pImpl->mLocationCode.clear();
    pImpl->mPhaseHint.clear();
    pImpl->mAlgorithm = "unspecified";
    pImpl->mTime = 0;
    pImpl->mIdentifier = 0;
    pImpl->mPolarity = Pick::Polarity::UNKNOWN;
    pImpl->mHaveTime = false;
    pImpl->mHaveIdentifier = false;
}

/// Network
void Pick::setNetwork(const std::string &network)
{
    if (isEmpty(network)){throw std::invalid_argument("Network is empty");}
    pImpl->mNetwork = network;
}

std::string Pick::getNetwork() const
{
    if (!haveNetwork()){throw std::runtime_error("Network not set yet");}
    return pImpl->mNetwork;
}

bool Pick::haveNetwork() const noexcept
{
    return !pImpl->mNetwork.empty();
}

/// Station
void Pick::setStation(const std::string &station)
{
    if (isEmpty(station)){throw std::invalid_argument("Station is empty");}
    pImpl->mStation = station;
}

std::string Pick::getStation() const
{
    if (!haveStation()){throw std::runtime_error("Station not set yet");}
    return pImpl->mStation;
}

bool Pick::haveStation() const noexcept
{
    return !pImpl->mStation.empty();
}

/// Channel
void Pick::setChannel(const std::string &channel)
{
    if (isEmpty(channel)){throw std::invalid_argument("Channel is empty");}
    pImpl->mChannel = channel;
}

std::string Pick::getChannel() const
{
    if (!haveChannel()){throw std::runtime_error("Channel not set yet");}
    return pImpl->mChannel;
}

bool Pick::haveChannel() const noexcept
{
    return !pImpl->mChannel.empty();
}

/// Location code
void Pick::setLocationCode(const std::string &location)
{
    if (isEmpty(location)){throw std::invalid_argument("location is empty");}
    pImpl->mLocationCode = location;
}

std::string Pick::getLocationCode() const
{
    if (!haveLocationCode())
    {
        throw std::runtime_error("Location code not set yet");
    }
    return pImpl->mLocationCode;
}

bool Pick::haveLocationCode() const noexcept
{
    return !pImpl->mLocationCode.empty();
}

/// Pick time
void Pick::setTime(const double time) noexcept
{
    pImpl->mTime = time;
    pImpl->mHaveTime = true;
}

double Pick::getTime() const
{
    if (!haveTime()){throw std::runtime_error("Time not yet set");}
    return pImpl->mTime;
}

bool Pick::haveTime() const noexcept
{
    return pImpl->mHaveTime;
}

/// Pick identifier
void Pick::setIdentifier(const uint64_t id) noexcept
{
    pImpl->mIdentifier = id;
    pImpl->mHaveIdentifier = true;
}

uint64_t Pick::getIdentifier() const
{
    if (!haveIdentifier()){throw std::runtime_error("Identifier not yet set");}
    return pImpl->mIdentifier;
}

bool Pick::haveIdentifier() const noexcept
{
    return pImpl->mHaveIdentifier;
}

/// Polarity
void Pick::setPolarity(const Pick::Polarity polarity) noexcept
{
    pImpl->mPolarity = polarity;
}

Pick::Polarity Pick::getPolarity() const noexcept
{
    return pImpl->mPolarity;
}

/// Algorithm
void Pick::setAlgorithm(const std::string &algorithm) noexcept
{
    pImpl->mAlgorithm = algorithm;
}

std::string Pick::getAlgorithm() const noexcept
{
    return pImpl->mAlgorithm;
} 

/// Phase hint
void Pick::setPhaseHint(const std::string &phaseHint) noexcept
{
    pImpl->mPhaseHint = phaseHint;
}

std::string Pick::getPhaseHint() const noexcept
{
    return pImpl->mPhaseHint;
}

/// Create JSON
std::string Pick::toJSON(const int nIndent) const
{
    auto obj = toJSONObject(*this);
    return obj.dump(nIndent);
}

/// Create CBOR
std::string Pick::toCBOR() const
{
    auto obj = toJSONObject(*this);
    auto v = nlohmann::json::to_cbor(obj);
    std::string result(v.begin(), v.end());
    return result; 
}

/// From JSON
void Pick::fromJSON(const std::string &message)
{
    *this = fromJSONMessage(message);
}

/// From CBOR
void Pick::fromCBOR(const std::string &data)
{
    fromCBOR(reinterpret_cast<const uint8_t *> (data.data()), data.size());
}

void Pick::fromCBOR(const uint8_t *data, const size_t length)
{
    if (length == 0){throw std::invalid_argument("No data");}
    if (data == nullptr)
    {
        throw std::invalid_argument("data is NULL");
    }
    *this = fromCBORMessage(data, length);
}

/// Copy this class
std::unique_ptr<URTS::MessageFormats::IMessage> Pick::clone() const
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Pick> (*this);
    return result;
}

/// Create an instance of this class 
std::unique_ptr<URTS::MessageFormats::IMessage>
    Pick::createInstance() const noexcept
{
    std::unique_ptr<MessageFormats::IMessage> result
        = std::make_unique<MessageFormats::Pick> ();
    return result;
}

/// Message type
std::string Pick::getMessageType() const noexcept
{
    return MESSAGE_TYPE;
}
