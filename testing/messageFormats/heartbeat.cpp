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
    const std::string moduleName = "heartbeatTest";
    const std::string hostName = "localhost";
    auto status = HeartbeatStatus::ALIVE;
    const std::string timeStamp("2021-10-11 21:36:42.090");
    // Check the defaults
    EXPECT_EQ(heartbeat.getModule(), "UNKNOWN");
    EXPECT_EQ(heartbeat.getHostName(), boost::asio::ip::host_name());
    EXPECT_EQ(heartbeat.getMessageType(), MESSAGE_TYPE); 
    EXPECT_EQ(heartbeat.getStatus(), HeartbeatStatus::UNKNOWN);
    EXPECT_TRUE(!heartbeat.getTimeStamp().empty());

    heartbeat.setStatus(status);
    EXPECT_NO_THROW(heartbeat.setModule(moduleName));
    EXPECT_NO_THROW(heartbeat.setHostName(hostName));
    EXPECT_NO_THROW(heartbeat.setTimeStamp(timeStamp));
    Heartbeat heartbeatCopy(heartbeat);

    EXPECT_EQ(heartbeatCopy.getMessageType(), MESSAGE_TYPE);
    EXPECT_EQ(heartbeatCopy.getModule(),      moduleName);
    EXPECT_EQ(heartbeatCopy.getHostName(),    hostName);
    EXPECT_EQ(heartbeatCopy.getTimeStamp(),   timeStamp);
    EXPECT_EQ(heartbeatCopy.getStatus(),      status);

    heartbeat.clear();
    auto json = heartbeatCopy.toJSON(4);
    heartbeat.fromJSON(json);
    EXPECT_EQ(heartbeat.getMessageType(), MESSAGE_TYPE);
    EXPECT_EQ(heartbeat.getModule(),      moduleName);
    EXPECT_EQ(heartbeat.getHostName(),    hostName);
    EXPECT_EQ(heartbeat.getTimeStamp(),   timeStamp);
    EXPECT_EQ(heartbeat.getStatus(),      status);

    heartbeat.clear();
    auto cbor = heartbeatCopy.toCBOR();
    heartbeat.fromCBOR(cbor);
    EXPECT_EQ(heartbeat.getMessageType(), MESSAGE_TYPE);
    EXPECT_EQ(heartbeat.getModule(),      moduleName);
    EXPECT_EQ(heartbeat.getHostName(),    hostName);
    EXPECT_EQ(heartbeat.getTimeStamp(),   timeStamp);
    EXPECT_EQ(heartbeat.getStatus(),      status);

    heartbeat.clear();
    auto msg = heartbeatCopy.toMessage();
    heartbeat.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(heartbeat.getMessageType(), MESSAGE_TYPE);
    EXPECT_EQ(heartbeat.getModule(),      moduleName);
    EXPECT_EQ(heartbeat.getHostName(),    hostName);
    EXPECT_EQ(heartbeat.getTimeStamp(),   timeStamp);
    EXPECT_EQ(heartbeat.getStatus(),      status);
}

}
