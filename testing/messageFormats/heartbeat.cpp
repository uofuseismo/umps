#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <limits>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/host_name.hpp>
#include "umps/proxyBroadcasts/heartbeat/status.hpp"
#include <gtest/gtest.h>
namespace
{

#define MESSAGE_TYPE "UMPS::ProxyBroadcasts::Heartbeat::Status"

using namespace UMPS::ProxyBroadcasts::Heartbeat;

TEST(HeartbeatTest, Status)
{
    Status status;
    const std::string moduleName = "heartbeatTest";
    const std::string hostName = "localhost";
    auto moduleStatus = ModuleStatus::Alive;
    const std::string timeStamp("2021-10-11T21:36:42.090");
    // Check the defaults
    EXPECT_EQ(status.getModule(), "unknown");
    EXPECT_EQ(status.getHostName(), boost::asio::ip::host_name());
    EXPECT_EQ(status.getMessageType(), MESSAGE_TYPE); 
    EXPECT_EQ(status.getModuleStatus(), ModuleStatus::Unknown);
    EXPECT_TRUE(!status.getTimeStamp().empty());

    status.setModuleStatus(moduleStatus);
    EXPECT_NO_THROW(status.setModule(moduleName));
    EXPECT_NO_THROW(status.setHostName(hostName));
    EXPECT_NO_THROW(status.setTimeStamp(timeStamp));
    Status statusCopy(status);

    EXPECT_EQ(statusCopy.getMessageType(),  MESSAGE_TYPE);
    EXPECT_EQ(statusCopy.getModule(),       moduleName);
    EXPECT_EQ(statusCopy.getHostName(),     hostName);
    EXPECT_EQ(statusCopy.getTimeStamp(),    timeStamp);
    EXPECT_EQ(statusCopy.getModuleStatus(), moduleStatus);

    status.clear();
    auto json = statusCopy.toJSON(4);
    status.fromJSON(json);
    EXPECT_EQ(status.getMessageType(),  MESSAGE_TYPE);
    EXPECT_EQ(status.getModule(),       moduleName);
    EXPECT_EQ(status.getHostName(),     hostName);
    EXPECT_EQ(status.getTimeStamp(),    timeStamp);
    EXPECT_EQ(status.getModuleStatus(), moduleStatus);

    status.clear();
    auto cbor = statusCopy.toCBOR();
    status.fromCBOR(cbor);
    EXPECT_EQ(status.getMessageType(),  MESSAGE_TYPE);
    EXPECT_EQ(status.getModule(),       moduleName);
    EXPECT_EQ(status.getHostName(),     hostName);
    EXPECT_EQ(status.getTimeStamp(),    timeStamp);
    EXPECT_EQ(status.getModuleStatus(), moduleStatus);

    status.clear();
    auto msg = statusCopy.toMessage();
    status.fromMessage(msg.data(), msg.size());
    EXPECT_EQ(status.getMessageType(),  MESSAGE_TYPE);
    EXPECT_EQ(status.getModule(),       moduleName);
    EXPECT_EQ(status.getHostName(),     hostName);
    EXPECT_EQ(status.getTimeStamp(),    timeStamp);
    EXPECT_EQ(status.getModuleStatus(), moduleStatus);

    const std::string timeStamp1("2021-10-11T21:36:42.091");
    status.setTimeStamp(timeStamp1);
    EXPECT_TRUE(status > statusCopy);

    // Test an edge case
    const std::string timeStamp3("2021-10-11T21:36:59.999");
    const std::string timeStamp4("2021-10-11T21:36:59.999");
    EXPECT_NO_THROW(status.setTimeStamp(timeStamp3));
    EXPECT_NO_THROW(statusCopy.setTimeStamp(timeStamp4));
    EXPECT_FALSE(status > statusCopy);
}

}
