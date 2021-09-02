#include <iostream>
#include <functional>
#include <string>
#include "urts/messaging/requestRouter/router.hpp"
#include "urts/logging/stdout.hpp"
#include <gtest/gtest.h>
namespace
{

const std::string serverHost = "tcp://*:5555"; 
const std::string localHost  = "tcp://127.0.0.1:5555";

TEST(Messaging, RequestRouter)
{
    //std::shared_ptr<void *> context = std::make_shared<void *> (zmq_ctx_new()); //zmq::context_t context{1};    
    URTS::Logging::StdOut logger;
    logger.setLevel(URTS::Logging::Level::DEBUG);
}

}
