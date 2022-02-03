#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
//#include "umps/proxyServices/packetCache/proxy.hpp"
//#include "umps/proxyServices/packetCache/proxyOptions.hpp"
#include "umps/proxyServices/proxy.hpp"
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/proxyServices/packetCache/reply.hpp"
#include "umps/proxyServices/packetCache/replyOptions.hpp"
#include "umps/proxyServices/packetCache/request.hpp"
#include "umps/proxyServices/packetCache/requestOptions.hpp"
/*
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/messaging/routerDealer/replyOptions.hpp"
*/
#include "private/staticUniquePointerCast.hpp"
#include "umps/messageFormats/messages.hpp"
#include <gtest/gtest.h>

namespace
{

/*
const std::string frontendAddress = "tcp://127.0.0.1:5555";
const std::string backendAddress  = "tcp://127.0.0.1:5556";
const std::string network{"UU"};
const std::string station{"FORK"};
const std::string channel{"HHZ"};
const std::string locationCode{"01"};
const double samplingRate{100};
const std::vector<double> timeSeries{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const int nMessages = 10;
const int t0 = 0;
*/

TEST(Communication, PacketCache)
{

}

}
