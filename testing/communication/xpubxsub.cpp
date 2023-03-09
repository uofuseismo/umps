#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/standardOut.hpp"
//#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/xPublisherXSubscriber/subscriber.hpp"
#include "umps/messaging/xPublisherXSubscriber/subscriberOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxy.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisher.hpp"
#include "umps/messaging/xPublisherXSubscriber/publisherOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/text.hpp"
#include "umps/messageFormats/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

//const std::string serverHost = "tcp://*:5555"; 
//const std::string localHost  = "tcp://127.0.0.1:5555";
// Faces internal network (pub)
const std::string frontendAddress = "tcp://127.0.0.1:5555";
// Faces external network (sub)
const std::string backendAddress = "tcp://127.0.0.1:5556";
const uint64_t idBase = 100;
//using namespace UMPS::Messaging::PublisherSubscriber;
namespace XPubXSub = UMPS::Messaging::XPublisherXSubscriber;
namespace UAuth = UMPS::Authentication;
namespace UMF = UMPS::MessageFormats;

void proxy()
{
    XPubXSub::ProxyOptions options;
    UAuth::ZAPOptions zapOptions;
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(100);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(200);
    //options.setTopic(topic);
    options.setZAPOptions(zapOptions);
    // Make a logger
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    // Initialize the server
    XPubXSub::Proxy proxy(loggerPtr);
    proxy.initialize(options);
    // A thread runs the proxy
    std::thread t1(&XPubXSub::Proxy::start,
                   &proxy);
    // Main thread waits...
    std::this_thread::sleep_for(std::chrono::seconds(3));
    /// Main thread tells proxy to stop
    proxy.stop();
    t1.join();
}

/*
void publisher(int id)
{
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    UMPS::Messaging::PublisherSubscriber::Publisher publisher(loggerPtr);
    publisher.bind(frontendAddress);
    // Deal with the slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Define message to send
    UMPS::MessageFormats::Pick pick;
    pick.setTime(time);
    pick.setNetwork(network);
    pick.setStation(station);
    pick.setChannel(channel);
    pick.setLocationCode(locationCode);
    pick.setPhaseHint(phaseHint);
    pick.setPolarity(polarity);

    for (int i = 0; i < 10; ++i)
    {
        pick.setIdentifier(idBase + i);
        publisher.send(pick);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
*/

void publisher(int id)
{
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    XPubXSub::PublisherOptions options;
    options.setAddress(frontendAddress); 
    XPubXSub::Publisher publisher;
    EXPECT_NO_THROW(publisher.initialize(options));
    EXPECT_TRUE(publisher.isInitialized());
    // Deal with the slow joiner problem
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    // Define message to send
    UMPS::MessageFormats::Text text;
    for (int i = 0; i < 10; ++i)
    {
        text.setContents(std::to_string(idBase + i));
        publisher.send(text);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void subscriber()
{
    UMPS::Logging::StandardOut logger;
    logger.setLevel(UMPS::Logging::Level::INFO);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StandardOut> (logger);
    std::unique_ptr<UMPS::MessageFormats::IMessage> textMessageType
        = std::make_unique<UMPS::MessageFormats::Text> ();
    UMPS::MessageFormats::Messages messageTypes;
    messageTypes.add(textMessageType);

    XPubXSub::SubscriberOptions options;
    options.setAddress(backendAddress);
    options.setMessageTypes(messageTypes);
    XPubXSub::Subscriber subscriber(loggerPtr);
    subscriber.initialize(options);
    //subscriber.connect(backendAddress);
    //subscriber.addSubscription(pickMessageType);

    for (int i = 0; i < 10; ++i)
    {
        auto message = subscriber.receive();
        auto pickMessage
            = UMF::static_unique_pointer_cast<UMPS::MessageFormats::Text>
              (std::move(message));
        EXPECT_EQ(pickMessage->getContents(),
                  std::to_string(idBase + i));
    }
}

TEST(Messaging, xPubxSubWithProxy)
{
    // Create the proxy then wait before connecting subscribers/publishers
    auto proxyThread  = std::thread(proxy);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // Create the consumers so messages aren't created before the recipients
    // are created
    auto subscriberThread1 = std::thread(subscriber);
    auto subscriberThread2 = std::thread(subscriber);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Now create a publishers
    auto publisherThread = std::thread(publisher, 1);
    //std::this_thread::sleep_for(std::chrono::milliseconds(10));
 
    proxyThread.join();
    subscriberThread1.join();
    subscriberThread2.join();
    publisherThread.join();
}

}
