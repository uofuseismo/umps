#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messageFormats/pick.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

const std::string serverHost = "tcp://*:5555"; 
const std::string localHost  = "tcp://127.0.0.1:5555";
//const std::string localHost = "inproc://a"; //{"inproc://#1"};
//const std::string localHost = "ipc://*";
using namespace UMPS::Messaging::PublisherSubscriber;

/*
template<typename TO, typename FROM>
std::unique_ptr<TO> static_unique_pointer_cast (std::unique_ptr<FROM> &&old)
{
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
}
*/

/*
void publish(std::shared_ptr<void *> *context)
{

try
{
    Publisher publisher;// *context);
    publisher.bind(serverHost);

    UMPS::MessageFormats::Pick pick;
    pick.setIdentifier(4043);
    pick.setTime(600);
    pick.setNetwork("UU"); 
    pick.setStation("NOQ");
    pick.setChannel("EHZ");
    pick.setLocationCode("01");
    pick.setPhaseHint("P");
    
int nSend = 0;
int maxSend = 1;
    while (true)
    {
        if (nSend == maxSend){break;}
        std::cout << "Sending..." << std::endl;
        publisher.send(pick);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        nSend = nSend + 1;
    }
}
catch (const std::exception &e)
{
 std::cerr << e.what() << std::endl;
}
}

void subscribe(std::shared_ptr<void *> *context)
{
    Subscriber subscriber; // *context);//(context); 
    subscriber.connect(localHost);
try
{
std::cout << "get it" << std::endl;
    auto message = subscriber.receive();
std::cout << "got it" << std::endl;
}
catch (const std::exception &e)
{
std::cerr << e.what() << std::endl;
}
}
*/

TEST(Messaging, PubSub)
{
    //std::shared_ptr<void *> context = std::make_shared<void *> (zmq_ctx_new()); //zmq::context_t context{1};    
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr = std::make_shared<UMPS::Logging::StdOut> (logger);
//       loggerPtr(std::make_shared<UMPS::Logging::StdOut> ());
//    loggerPtr->setLevel(UMPS::Logging::Level::DEBUG);

    // Create publisher and bind
//    Publisher publisher(loggerPtr);
//    publisher.bind(serverHost);

    // Create a subscriber and subscribe to all messages
    Subscriber subscriber(loggerPtr);
    subscriber.connect(localHost);
    std::unique_ptr<UMPS::MessageFormats::IMessage> pickMessageType
        = std::make_unique<UMPS::MessageFormats::Pick> ();
    subscriber.addSubscription(pickMessageType);
    //sleep(1);
    // Create publisher and bind
    Publisher publisher(loggerPtr);
    publisher.bind(serverHost);//localHost);
    // Give the publisher a chance to bind to the port
    std::this_thread::sleep_for(std::chrono::seconds(1)); //sleep(1);
    // Define message to send
    UMPS::MessageFormats::Pick pick;
    pick.setIdentifier(4043);
    pick.setTime(600);
    pick.setNetwork("UU"); 
    pick.setStation("NOQ");
    pick.setChannel("EHZ");
    pick.setLocationCode("01");
    pick.setPhaseHint("P");
    // Send it
    publisher.send(pick);

//    auto pickMessage = std::make_unique<UMPS::MessageFormats::Pick> ();
    auto message = subscriber.receive(); //dynamic_cast<UMPS::MessageFormats::Pick *> (subscriber.receive().get());
    auto pickMessage
        = static_unique_pointer_cast<UMPS::MessageFormats::Pick>
          (std::move(message));
    //std::cout << pickMessage->toJSON() << std::endl;
    EXPECT_NEAR(pickMessage->getTime(), pick.getTime(), 1.e-10);
    EXPECT_EQ(pickMessage->getIdentifier(),   pick.getIdentifier());
    EXPECT_EQ(pickMessage->getNetwork(),      pick.getNetwork());
    EXPECT_EQ(pickMessage->getStation(),      pick.getStation());
    EXPECT_EQ(pickMessage->getChannel(),      pick.getChannel());
    EXPECT_EQ(pickMessage->getLocationCode(), pick.getLocationCode());
    EXPECT_EQ(pickMessage->getPhaseHint(),    pick.getPhaseHint());
    EXPECT_EQ(pickMessage->getPolarity(),     pick.getPolarity());
//    auto pickMessage = std::static_unique_pointer_cast<UMPS::MessageFormats::Pick> (message);
    
    //std::cout << pickMessage->toJSON() << std::endl;
    /*
    //const std::string localHost("tcp://127.0.0.1:5555");
    //const std::string localHost("inproc://#1");
   
    // Start all subscriber sockets prior to publish sockets
    auto thread1 = std::async(std::launch::async, subscribe, &context);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
 
    auto thread2 = std::async(std::launch::async, publish,   &context);//, localHost);

    //std::thread thread1(publish);//, &context);
    //std::thread thread2(subscribe);//, &context);

    //thread1.join();
    //thread2.join();
    thread1.wait();
    thread2.wait();
 */
//getchar();
/*
    Publisher publisher;
    Subscriber subscriber;
    EXPECT_NO_THROW(publisher.bind(localHost));
    EXPECT_NO_THROW(subscriber.connect(localHost));
    //sleep(1);

    UMPS::MessageFormats::Pick pick;
    pick.setIdentifier(4043);
    pick.setTime(600);
    pick.setNetwork("UU"); 
    pick.setStation("NOQ");
    pick.setChannel("EHZ");
    pick.setLocationCode("01");
    pick.setPhaseHint("P");
    publisher.send(pick);
*/
}

}
