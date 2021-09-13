#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "urts/logging/stdout.hpp"
#include "urts/messaging/publisherSubscriber/publisher.hpp"
#include "urts/messaging/publisherSubscriber/subscriber.hpp"
#include "urts/messaging/publisherSubscriber/proxy.hpp"
#include "urts/messageFormats/pick.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

const std::string serverHost = "tcp://*:5555"; 
const std::string localHost  = "tcp://127.0.0.1:5555";
const std::string frontendAddress = "tcp://127.0.0.1:5555";
const std::string backendAddress = "tcp://127.0.0.1:5556";
const std::string topic = "proxyTest";
//const std::string localHost = "inproc://a"; //{"inproc://#1"};
//const std::string localHost = "ipc://*";
using namespace URTS::Messaging::PublisherSubscriber;

void proxy()
{
    // Make a logger
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::DEBUG); 
    std::shared_ptr<URTS::Logging::ILog> loggerPtr
        = std::make_shared<URTS::Logging::StdOut> (logger);
    // Initialize the server
    URTS::Messaging::PublisherSubscriber::Proxy proxy(loggerPtr);
    proxy.initialize(frontendAddress, backendAddress, topic);

    std::thread t1(&URTS::Messaging::PublisherSubscriber::Proxy::start,
                   &proxy);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    proxy.stop();
std::cout << "okay" << std::endl;
/*
    try
    {
        proxy.stop();
    }
    catch (const std::exception &e)
    {
       std::cerr << e.what() << std::endl;
    }
*/
    t1.join();
/*
    server.bind(serverHost);
    server.setCallback(std::bind(&ProcessData::process,
                                 &pStruct, //process,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3));
*/
}

TEST(Messaging, xPubxSubWithProxy)
{
    auto proxyThread  = std::thread(proxy);

    proxyThread.join();
}

}
