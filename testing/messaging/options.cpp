#include <string>
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/socketOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/publisherSubscriber/publisherOptions.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messageFormats/text.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::Messaging;
namespace UAuth = UMPS::Authentication;

TEST(Messaging, SocketOptions)
{
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessage
        = std::make_unique<UMPS::MessageFormats::Text> ();
    UMPS::MessageFormats::Messages messageFormats;
    EXPECT_NO_THROW(messageFormats.add(textMessage));

    const std::string address{"tcp://127.0.0.1:5556"};
    const std::string routingID{"temp_client"}; 
    const int sendHWM{1};
    const int recvHWM{2}; 
    const std::chrono::milliseconds sendTimeOut{31};
    const std::chrono::milliseconds recvTimeOut{32};
    const std::chrono::milliseconds lingerPeriod{-5}; // Resolve to -1
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();

    SocketOptions options;
    EXPECT_NO_THROW(options.setAddress(address)); 
    EXPECT_NO_THROW(options.setSendHighWaterMark(sendHWM));
    EXPECT_NO_THROW(options.setReceiveHighWaterMark(recvHWM));
    options.setSendTimeOut(sendTimeOut);
    options.setReceiveTimeOut(recvTimeOut);
    options.setZAPOptions(zapOptions);
    options.setRoutingIdentifier(routingID);
    options.setLingerPeriod(lingerPeriod);
    EXPECT_NO_THROW(options.setMessageFormats(messageFormats));

    SocketOptions copy(options);
    EXPECT_EQ(copy.getAddress(), address);
    EXPECT_EQ(copy.getSendHighWaterMark(), sendHWM);
    EXPECT_EQ(copy.getReceiveHighWaterMark(), recvHWM);
    EXPECT_EQ(copy.getSendTimeOut(), sendTimeOut);
    EXPECT_EQ(copy.getReceiveTimeOut(), recvTimeOut);
    EXPECT_EQ(copy.getRoutingIdentifier(), routingID);
    EXPECT_EQ(copy.getLingerPeriod(), std::chrono::milliseconds {-1});
    EXPECT_EQ(copy.getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
    EXPECT_TRUE(copy.getMessageFormats().contains(textMessage));

    options.clear();
    EXPECT_EQ(options.getReceiveHighWaterMark(), 0);
    EXPECT_EQ(options.getSendHighWaterMark(), 0);
    EXPECT_EQ(options.getReceiveTimeOut(), std::chrono::milliseconds {-1});
    EXPECT_EQ(options.getSendTimeOut(), std::chrono::milliseconds {-1});
    EXPECT_EQ(options.getLingerPeriod(), std::chrono::milliseconds {-1});
    EXPECT_FALSE(options.haveRoutingIdentifier());
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Grasslands); 
}

TEST(Messaging, PubSubPublisherOptions)
{
    const std::string address = "tcp://127.0.0.1:5556";
    const int highWaterMark = 120;
    const int zero = 0;
    const std::chrono::milliseconds timeOut{10};
    PublisherSubscriber::PublisherOptions options;
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setSendHighWaterMark(highWaterMark));
    EXPECT_NO_THROW(options.setSendTimeOut(timeOut));

    PublisherSubscriber::PublisherOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getSendHighWaterMark(), highWaterMark);
    EXPECT_EQ(optionsCopy.getSendTimeOut(), timeOut);

    options.clear();
    EXPECT_EQ(options.getSendHighWaterMark(), zero); 
}

TEST(Messaging, PubSubSubscriberOptions)
{
    UMPS::MessageFormats::Messages messageTypes;
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessage
        = std::make_unique<UMPS::MessageFormats::Text> ();
    std::unique_ptr<UMPS::MessageFormats::IMessage> packetMessage
        = std::make_unique<UMPS::MessageFormats::DataPacket<double>> ();
    EXPECT_NO_THROW(messageTypes.add(textMessage));
    EXPECT_NO_THROW(messageTypes.add(packetMessage));
    const std::string address = "tcp://127.0.0.1:5555";
    const int highWaterMark = 120;
    const std::chrono::milliseconds timeOut{10};
    const int zero = 0;
    PublisherSubscriber::SubscriberOptions options;
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setMessageTypes(messageTypes));
    EXPECT_NO_THROW(options.setReceiveHighWaterMark(highWaterMark));
    EXPECT_NO_THROW(options.setReceiveTimeOut(timeOut));

    PublisherSubscriber::SubscriberOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getAddress(), address);
    EXPECT_EQ(optionsCopy.getReceiveHighWaterMark(), highWaterMark);
    EXPECT_EQ(optionsCopy.getReceiveTimeOut(), timeOut);
    EXPECT_TRUE(optionsCopy.haveMessageTypes());
    auto messagesBack = optionsCopy.getMessageTypes();
    EXPECT_TRUE(messagesBack.contains(textMessage));
    EXPECT_TRUE(messagesBack.contains(packetMessage));

    options.clear();
    EXPECT_EQ(options.getReceiveHighWaterMark(), zero); 
    EXPECT_EQ(options.getReceiveTimeOut(), std::chrono::milliseconds{-1});
    EXPECT_FALSE(options.haveMessageTypes());
}

TEST(Messaging, XPubXSubProxyOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const std::string backendAddress = "tcp://127.0.0.2:5556";
    //const std::string name = "testTopic";
    const int frontendHWM = 100;
    const int backendHWM = 200;
    const int zero = 0;
    XPublisherXSubscriber::ProxyOptions options;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(backendHWM);
    //options.setName(name);
  
    XPublisherXSubscriber::ProxyOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getFrontendAddress(), frontendAddress);
    EXPECT_EQ(optionsCopy.getBackendAddress(), backendAddress);
    EXPECT_EQ(optionsCopy.getFrontendHighWaterMark(), frontendHWM);
    EXPECT_EQ(optionsCopy.getBackendHighWaterMark(), backendHWM);
    //EXPECT_EQ(optionsCopy.getName(), name);
   
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

TEST(Messaging, RequestRouterRequestOptions)
{
    RequestRouter::RequestOptions options;
    int hwm = 240;
    const std::string address = "tcp://127.0.0.2:5556";
    const std::chrono::milliseconds timeOut{120};
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessage
        = std::make_unique<UMPS::MessageFormats::Text> (); 
    //UMPS::MessageFormats::Messages messageFormats;
    //messageFormats.add(textMessage);
    EXPECT_NO_THROW(options.setHighWaterMark(hwm));
    EXPECT_NO_THROW(options.setZAPOptions(zapOptions));
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setTimeOut(timeOut));
    EXPECT_NO_THROW(options.addMessageFormat(textMessage));
    
    RequestRouter::RequestOptions optionsCopy(options); 
    EXPECT_EQ(options.getHighWaterMark(), hwm);
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Strawhouse);
    EXPECT_EQ(options.getAddress(), address);
    EXPECT_EQ(options.getTimeOut(), timeOut);
    EXPECT_TRUE(options.getMessageFormats().contains(textMessage));

    options.clear();
    EXPECT_EQ(options.getHighWaterMark(), 0); 
    const std::chrono::milliseconds negativeOne{-1};
    EXPECT_EQ(options.getTimeOut(), negativeOne);
}

TEST(Messaging, RouterDealerProxyOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const std::string backendAddress = "tcp://127.0.0.2:5556";
    //const std::string topic = "testTopic";
    const int frontendHWM = 100;
    const int backendHWM = 200;
    const int zero = 0;
    RouterDealer::ProxyOptions options;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(backendHWM);
    //options.setTopic(topic);
  
    RouterDealer::ProxyOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getFrontendAddress(), frontendAddress);
    EXPECT_EQ(optionsCopy.getBackendAddress(), backendAddress);
    EXPECT_EQ(optionsCopy.getFrontendHighWaterMark(), frontendHWM);
    EXPECT_EQ(optionsCopy.getBackendHighWaterMark(), backendHWM);
    //EXPECT_EQ(optionsCopy.getTopic(), topic);
   
    options.clear();
    EXPECT_EQ(options.getFrontendHighWaterMark(), zero);
    EXPECT_EQ(options.getBackendHighWaterMark(), zero);
}

TEST(Messaging, RouterDealerRequestOptions)
{
    RouterDealer::RequestOptions options;
    const int sendhwm{240};
    const int rcvhwm{250};
    const std::string address = "tcp://127.0.0.2:5556";
    const std::chrono::milliseconds sendTimeOut{120};
    const std::chrono::milliseconds receiveTimeOut{150};
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessage
        = std::make_unique<UMPS::MessageFormats::Text> (); 
    UMPS::MessageFormats::Messages messageFormats;
    messageFormats.add(textMessage);
    EXPECT_NO_THROW(options.setSendHighWaterMark(sendhwm));
    EXPECT_NO_THROW(options.setReceiveHighWaterMark(rcvhwm));
    EXPECT_NO_THROW(options.setZAPOptions(zapOptions));
    EXPECT_NO_THROW(options.setAddress(address));
    options.setSendTimeOut(sendTimeOut);
    options.setReceiveTimeOut(receiveTimeOut);
    EXPECT_NO_THROW(options.setMessageFormats(messageFormats));
        
    RouterDealer::RequestOptions optionsCopy(options); 
    EXPECT_EQ(options.getSendHighWaterMark(), sendhwm);
    EXPECT_EQ(options.getReceiveHighWaterMark(), rcvhwm);
    EXPECT_EQ(options.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Strawhouse);
    EXPECT_EQ(options.getAddress(), address);
    EXPECT_EQ(options.getSendTimeOut(), sendTimeOut);
    EXPECT_EQ(options.getReceiveTimeOut(), receiveTimeOut);
    EXPECT_TRUE(options.getMessageFormats().contains(textMessage));

    options.clear();
    EXPECT_EQ(options.getSendHighWaterMark(), 0); 
    EXPECT_EQ(options.getReceiveHighWaterMark(), 0);
    const std::chrono::milliseconds negativeOne{-1};
    EXPECT_EQ(options.getSendTimeOut(), std::chrono::milliseconds {0});
    EXPECT_EQ(options.getReceiveTimeOut(), negativeOne);
}

TEST(Messaging, RouterDealerReplyOptions)
{
    RouterDealer::ReplyOptions options; 
    const int sendhwm{240};
    const int recvhwm{250}; 
    const std::string address = "tcp://127.0.0.2:5556";
    const std::string routingIdentifier{"tempIdentifier"};
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseClient();
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessage
        = std::make_unique<UMPS::MessageFormats::Text> ();
    EXPECT_NO_THROW(options.setSendHighWaterMark(sendhwm));
    EXPECT_NO_THROW(options.setReceiveHighWaterMark(recvhwm));
    EXPECT_NO_THROW(options.setZAPOptions(zapOptions));
    EXPECT_NO_THROW(options.setAddress(address));
    EXPECT_NO_THROW(options.setRoutingIdentifier(routingIdentifier));
    //EXPECT_NO_THROW(options.addMessageFormat(textMessage));
    
    RouterDealer::ReplyOptions optionsCopy(options); 
    EXPECT_EQ(optionsCopy.getSendHighWaterMark(), sendhwm);
    EXPECT_EQ(optionsCopy.getReceiveHighWaterMark(), recvhwm);
    EXPECT_EQ(optionsCopy.getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::Strawhouse);
    EXPECT_EQ(optionsCopy.getAddress(), address);
    //EXPECT_TRUE(options.getMessageFormats().contains(textMessage));
    EXPECT_EQ(optionsCopy.getRoutingIdentifier(), routingIdentifier);

    options.clear();
    EXPECT_EQ(options.getSendHighWaterMark(), 0);
    EXPECT_EQ(options.getReceiveHighWaterMark(), 0);
    EXPECT_FALSE(options.haveRoutingIdentifier());
}

}
