#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include "umps/messageFormats/pick.hpp"
#include <gtest/gtest.h>
namespace
{

#define MESSAGE_TYPE "UMPS::MessageFormats::Pick"

using namespace UMPS::MessageFormats;

TEST(PickTest, Pick)
{
    Pick pick;
    const uint64_t pickID = 84823;
    const std::string network = "UU";
    const std::string station = "MOUT";
    const std::string channel = "EHZ";
    const std::string locationCode = "01";
    const std::string phaseHint = "P";
    const std::string algorithm = "autoPicker";
    const double time = 500;
    auto polarity = Pick::Polarity::UP;

    pick.setIdentifier(pickID);
    pick.setTime(time);
    EXPECT_NO_THROW(pick.setNetwork(network));
    EXPECT_NO_THROW(pick.setStation(station));
    EXPECT_NO_THROW(pick.setChannel(channel));
    EXPECT_NO_THROW(pick.setLocationCode(locationCode));
    EXPECT_EQ(pick.getMessageType(), MESSAGE_TYPE);
    pick.setPolarity(polarity);
    pick.setPhaseHint(phaseHint);
    pick.setAlgorithm(algorithm);

    Pick pickCopy(pick);
    
    EXPECT_EQ(pickCopy.getIdentifier(), pickID);
    EXPECT_NEAR(pickCopy.getTime(), time, 1.e-10);
    EXPECT_EQ(pickCopy.getNetwork(), network);
    EXPECT_EQ(pickCopy.getStation(), station);
    EXPECT_EQ(pickCopy.getChannel(), channel);
    EXPECT_EQ(pickCopy.getLocationCode(), locationCode);
    EXPECT_EQ(pickCopy.getPolarity(), polarity);
    EXPECT_EQ(pickCopy.getPhaseHint(), phaseHint);
    EXPECT_EQ(pickCopy.getAlgorithm(), algorithm);

    auto messageJSON = pickCopy.toJSON(4);

    pick.clear();
    EXPECT_EQ(pick.getPolarity(), Pick::Polarity::UNKNOWN);
    EXPECT_TRUE(pick.getPhaseHint().empty());
    EXPECT_EQ(pick.getAlgorithm(), "unspecified");

    pick.fromJSON(messageJSON);
    EXPECT_EQ(pick.getIdentifier(), pickID);
    EXPECT_NEAR(pick.getTime(), time, 1.e-10);
    EXPECT_EQ(pick.getNetwork(), network);
    EXPECT_EQ(pick.getStation(), station);
    EXPECT_EQ(pick.getChannel(), channel);
    EXPECT_EQ(pick.getLocationCode(), locationCode);
    EXPECT_EQ(pick.getPolarity(), polarity);
    EXPECT_EQ(pick.getPhaseHint(), phaseHint);
    EXPECT_EQ(pick.getAlgorithm(), algorithm);

    pick.clear();
    auto cbor = pickCopy.toCBOR();
    EXPECT_NO_THROW(pick.fromCBOR(cbor));
    EXPECT_EQ(pick.getIdentifier(), pickID);
    EXPECT_NEAR(pick.getTime(), time, 1.e-10);
    EXPECT_EQ(pick.getNetwork(), network);
    EXPECT_EQ(pick.getStation(), station);
    EXPECT_EQ(pick.getChannel(), channel);
    EXPECT_EQ(pick.getLocationCode(), locationCode);
    EXPECT_EQ(pick.getPolarity(), polarity);
    EXPECT_EQ(pick.getPhaseHint(), phaseHint);
    EXPECT_EQ(pick.getAlgorithm(), algorithm);

    pick.clear();
    auto message = pickCopy.toMessage();
    EXPECT_NO_THROW(pick.fromMessage(message.data(), message.size()));
    EXPECT_EQ(pick.getIdentifier(), pickID);
    EXPECT_NEAR(pick.getTime(), time, 1.e-10);
    EXPECT_EQ(pick.getNetwork(), network);
    EXPECT_EQ(pick.getStation(), station);
    EXPECT_EQ(pick.getChannel(), channel);
    EXPECT_EQ(pick.getLocationCode(), locationCode);
    EXPECT_EQ(pick.getPolarity(), polarity);
    EXPECT_EQ(pick.getPhaseHint(), phaseHint);
    EXPECT_EQ(pick.getAlgorithm(), algorithm);
}

}
