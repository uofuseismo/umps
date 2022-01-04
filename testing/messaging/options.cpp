#include <string>
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/publisherOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messageFormats/pick.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Messaging;

TEST(Messaging, PubSubPublisherOptions)
{
    const std::string address = "tcp://127.0.0.1:5556";
    const int highWaterMark = 120;
    const int zero = 0;
    const std::chrono::milliseconds timeOut{10};
    PublisherSubscriber::PublisherOptions options;
    options.setAddress(address);
    options.setHighWaterMark(highWaterMark);
    options.setTimeOut(timeOut);

    PublisherSubscriber::PublisherOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), highWaterMark);
    EXPECT_EQ(optionsCopy.getTimeOut(), timeOut);

    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), zero); 
}

TEST(Messaging, PubSubSubscriberOptions)
{
    UMPS::MessageFormats::Messages messageTypes;
    std::unique_ptr<UMPS::MessageFormats::IMessage> pickMessage
        = std::make_unique<UMPS::MessageFormats::Pick> ();
    std::unique_ptr<UMPS::MessageFormats::IMessage> packetMessage
        = std::make_unique<UMPS::MessageFormats::DataPacket<double>> ();
    EXPECT_NO_THROW(messageTypes.add(pickMessage));
    EXPECT_NO_THROW(messageTypes.add(packetMessage));
    const std::string address = "tcp://127.0.0.1:5555";
    const int highWaterMark = 120;
    const std::chrono::milliseconds timeOut{10};
    const int zero = 0;
    PublisherSubscriber::SubscriberOptions options;
    options.setAddress(address);
    options.setHighWaterMark(highWaterMark);
    options.setMessageTypes(messageTypes);
    options.setTimeOut(timeOut);

    PublisherSubscriber::SubscriberOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), highWaterMark);
    EXPECT_EQ(optionsCopy.getTimeOut(), timeOut);
    EXPECT_TRUE(optionsCopy.haveMessageTypes());
    auto messagesBack = optionsCopy.getMessageTypes();
    EXPECT_TRUE(messagesBack.contains(pickMessage));
    EXPECT_TRUE(messagesBack.contains(packetMessage));

    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), zero); 
    EXPECT_EQ(options.getTimeOut(), std::chrono::milliseconds{-1});
    EXPECT_FALSE(options.haveMessageTypes());
}

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
    const std::chrono::milliseconds timeOut{10};
    const std::chrono::milliseconds negativeOne{-1};
    XPublisherXSubscriber::PublisherOptions options;
    options.setAddress(address);
    options.setHighWaterMark(highWaterMark);
    options.setTimeOut(timeOut);
  
    XPublisherXSubscriber::PublisherOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getHighWaterMark(), highWaterMark);
    EXPECT_EQ(optionsCopy.getTimeOut(), timeOut);
   
    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), zero);
    EXPECT_EQ(options.getTimeOut(), negativeOne);
}

}
