#include <iostream>
#include <string>
#include <chrono>
#include <boost/asio.hpp>
#include "umps/proxyBroadcasts/pick/publisherOptions.hpp"
#include "umps/proxyBroadcasts/pick/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/pick.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyBroadcasts::Pick;
namespace UAuth = UMPS::Authentication;

TEST(BroadcastsHeartbeat, SubscriberOptions)
{
    UMPS::MessageFormats::Pick pick;
    // Test defaults
    SubscriberOptions options;
    EXPECT_EQ(options.getTimeOut(), std::chrono::milliseconds{10});
    EXPECT_EQ(options.getHighWaterMark(), 8192);
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Grasslands);
    auto messageTypes = options.getSubscriberOptions().getMessageTypes();
    EXPECT_EQ(messageTypes.size(), 1);
    EXPECT_TRUE(messageTypes.contains(pick));
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
    EXPECT_TRUE(messageTypes.contains(pick));
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

}
