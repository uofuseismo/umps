#include <iostream>
#include <string>
#include <chrono>
#include <boost/asio.hpp>
#include "umps/proxyBroadcasts/heartbeat/publisherOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/subscriberOptions.hpp"
#include "umps/proxyBroadcasts/heartbeat/status.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyBroadcasts::Heartbeat;
namespace UAuth = UMPS::Authentication;

TEST(BroadcastsHeartbeat, SubscriberOptions)
{
    Status status;
    // Test defaults
    SubscriberOptions options;
    EXPECT_EQ(options.getTimeOut(), std::chrono::milliseconds{10});
    EXPECT_EQ(options.getHighWaterMark(), 1024);
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Grasslands);
    auto messageTypes = options.getSubscriberOptions().getMessageTypes();
    EXPECT_EQ(messageTypes.size(), 1);
    EXPECT_TRUE(messageTypes.contains(status));
    // Test fascade
    const std::string backEnd = "tcp://127.0.0.1:8081";
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    std::chrono::milliseconds timeOut{57};
    int hwm = 0;
    EXPECT_NO_THROW(options.setAddress(backEnd));
    EXPECT_NO_THROW(options.setTimeOut(timeOut));
    EXPECT_NO_THROW(options.setHighWaterMark(hwm));
    EXPECT_NO_THROW(options.setZAPOptions(zapOptions));

    SubscriberOptions optionsCopy(options);
    EXPECT_EQ(optionsCopy.getAddress(), backEnd);
    EXPECT_EQ(optionsCopy.getTimeOut(), timeOut);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), hwm);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    messageTypes = optionsCopy.getSubscriberOptions().getMessageTypes();
    EXPECT_EQ(messageTypes.size(), 1); 
    EXPECT_TRUE(messageTypes.contains(status));
}

TEST(BroadcastHeartbeat, PublisherOptions)
{
    // Test defaults
    PublisherOptions options;
    std::chrono::milliseconds oneSecond{1000};
    EXPECT_EQ(options.getHighWaterMark(), 8192);
    EXPECT_EQ(options.getTimeOut(), oneSecond);
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Grasslands);
    // Test facade
    const std::string frontEnd = "tcp://127.0.0.1:8082";
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    int hwm = 1024;
    std::chrono::milliseconds twoSeconds{2000};
    EXPECT_NO_THROW(options.setAddress(frontEnd));
    EXPECT_NO_THROW(options.setHighWaterMark(hwm));
    options.setTimeOut(twoSeconds);
    EXPECT_NO_THROW(options.setZAPOptions(zapOptions));

    PublisherOptions optionsCopy(options);
    EXPECT_EQ(optionsCopy.getAddress(), frontEnd);
    EXPECT_EQ(optionsCopy.getTimeOut(), twoSeconds);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), hwm);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
}

TEST(BroadcastHeartbeat, Status)
{
    Status status;
    const std::string module = "testModule";
    const std::string hostName = "testComputer";
    const ModuleStatus moduleStatus = ModuleStatus::Alive;
    std::string referenceHostName = boost::asio::ip::host_name();
    std::string timeStamp{"2022-11-01T00:01:02.300"}; 

    EXPECT_NO_THROW(status.setModule(module));
    EXPECT_NO_THROW(status.setModuleStatus(moduleStatus));
    EXPECT_NO_THROW(status.setHostName(hostName));
    EXPECT_NO_THROW(status.setTimeStamp(timeStamp));

    auto message = status.toMessage();
    Status statusCopy;
    statusCopy.fromMessage(message.data(), message.size());

    EXPECT_EQ(statusCopy.getModule(), module);
    EXPECT_EQ(statusCopy.getModuleStatus(), moduleStatus);
    EXPECT_EQ(statusCopy.getHostName(), hostName);
    EXPECT_EQ(statusCopy.getTimeStamp(), timeStamp);
    
    // Check defaults
    status.clear();
    EXPECT_EQ(status.getModuleStatus(), ModuleStatus::Unknown);
    EXPECT_EQ(status.getHostName(), referenceHostName);
}

}
