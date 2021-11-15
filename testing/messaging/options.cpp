#include <string>
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Messaging;

TEST(Messaging, XPubXSubProxyOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const std::string backendAddress = "tcp://127.0.0.2:5556";
    const std::string topic = "testTopic";
    const int frontendHWM = 100;
    const int backendHWM = 200;
    const int zero = 0;
    XPublisherXSubscriber::ProxyOptions options;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(backendHWM);
    options.setTopic(topic);
  
    XPublisherXSubscriber::ProxyOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getFrontendAddress(), frontendAddress);
    EXPECT_EQ(optionsCopy.getBackendAddress(), backendAddress);
    EXPECT_EQ(optionsCopy.getFrontendHighWaterMark(), frontendHWM);
    EXPECT_EQ(optionsCopy.getBackendHighWaterMark(), backendHWM);
    EXPECT_EQ(optionsCopy.getTopic(), topic);
   
    options.clear();
    EXPECT_EQ(options.getFrontendHighWaterMark(), zero);
    EXPECT_EQ(options.getBackendHighWaterMark(), zero);
}

TEST(Messaging, XPubXSubPublisherOptions)
{
    const std::string address = "tcp://127.0.0.1:5555";
    const int highWaterMark = 120;
    const int zero = 0;
    XPublisherXSubscriber::PublisherOptions options;
    options.setAddress(address);
    options.setHighWaterMark(highWaterMark);
  
    XPublisherXSubscriber::PublisherOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), highWaterMark);
   
    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), zero);
}

}
