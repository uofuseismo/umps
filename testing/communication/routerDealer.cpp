#include <string>
#include <future>
#include <chrono>
#include <vector>
#include <thread>
#include <zmq.hpp>
#include "umps/logging/stdout.hpp"
#include "umps/messaging/routerDealer/proxy.hpp"
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
//#include "umps/messaging/routerDealer/reply.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/pick.hpp"
#include "private/staticUniquePointerCast.hpp"
#include <gtest/gtest.h>
namespace
{

// Faces internal network (sub)
const std::string frontendAddress = "tcp://127.0.0.1:5555";
// Faces external network (pub)
const std::string backendAddress = "tcp://127.0.0.1:5556";

}
