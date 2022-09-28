#include <thread>
#include <chrono>
#include "umps/services/command/remoteProxy.hpp"
#include "umps/services/command/remoteProxyOptions.hpp"
#include "umps/services/command/remoteRequestor.hpp"
#include "umps/services/command/remoteRequestorOptions.hpp"
#include "umps/services/command/remoteService.hpp"
#include "umps/services/command/remoteServiceOptions.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messageFormats/text.hpp"
#include <gtest/gtest.h>

#define FRONTEND "tcp://127.0.0.1:5000"
#define BACKEND  "tcp://127.0.0.1:5001"

namespace
{

using namespace UMPS::Services::Command;

/*
class ResponseMessage : public UMPS::MessageFormats::IMessage
{
public:
    [[nodiscard]] std::string getMessageType() const noexcept final
    {
        return "TestResponse";
    }
    void fromMessage(const std::string &message)
    {
        fromMessage(message.c_str(), message.size()); 
    }
    void fromMessage(const char *, const size_t) final
    {
    }
};
*/

class Response
{
public:
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *data, size_t length)
    {
        auto text = std::make_unique<UMPS::MessageFormats::Text> ();
        text->setContents("Test result"); 
        return text;
    }
};

void proxy( )
{
    RemoteProxyOptions options;
    options.setFrontendAddress(FRONTEND);
    options.setBackendAddress(BACKEND);
    RemoteProxy proxy; 
    EXPECT_NO_THROW(proxy.initialize(options));
    proxy.start();
    std::this_thread::sleep_for(std::chrono::seconds {5});
    proxy.stop();
}

void respondor()
{
    // Give proxy a chance to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds {10});
}

void requestor()
{
    RemoteRequestor requestor;
    RemoteRequestorOptions options;
    options.setAddress(FRONTEND);
    // Give proxy and respondor a chance to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds {20});
    requestor.initialize(options);
}

TEST(RemoteCommand, RemoteCommand)
{
    auto proxyThread = std::thread(proxy); // Intermediary
    auto respondorThread = std::thread(respondor); // Setup person on end
    auto requestorThread = std::thread(requestor); // Ask last
     
    requestorThread.join();
    respondorThread.join();
    proxyThread.join();
}

}
