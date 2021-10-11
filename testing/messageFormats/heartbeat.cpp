#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/host_name.hpp>
#include "umps/messageFormats/heartbeat.hpp"
#include <gtest/gtest.h>
namespace
{

#define MESSAGE_TYPE "UMPS::MessageFormats::Heartbeat"

using namespace UMPS::MessageFormats;

TEST(HeartbeatTest, Heartbeat)
{
    Heartbeat heartbeat;
    auto hostName = heartbeat.getHostName();
    // Check the defaults
    EXPECT_EQ(heartbeat.getHostName(), boost::asio::ip::host_name());
    EXPECT_EQ(heartbeat.getMessageType(), MESSAGE_TYPE); 
    EXPECT_EQ(heartbeat.getStatus(), HeartbeatStatus::UNKNOWN);
    EXPECT_TRUE(!heartbeat.getTimeStamp().empty());
std::cout << heartbeat.getTimeStamp() << std::endl;
//std::cout << heartbeat.getHostName() << std::endl;
//std::cout << boost::asio::ip::address() << std::endl;
/*
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
*/
    EXPECT_EQ(heartbeat.getMessageType(), MESSAGE_TYPE);
/*
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

    auto message = pickCopy.toJSON(4);

    pick.clear();
*/
}

}
