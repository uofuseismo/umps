#include <thread>
#include <chrono>
#include "umps/services/command/remoteProxy.hpp"
#include "umps/services/command/remoteProxyOptions.hpp"
#include "umps/services/command/remoteRequestor.hpp"
#include "umps/services/command/remoteRequestorOptions.hpp"
#include "umps/services/command/remoteReplier.hpp"
#include "umps/services/command/remoteReplierOptions.hpp"
#include "umps/services/command/moduleDetails.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messageFormats/text.hpp"
#include "umps/logging/stdout.hpp"
#include <gtest/gtest.h>

#define FRONTEND "tcp://127.0.0.1:5000"
#define BACKEND  "tcp://127.0.0.1:5001"
#define MODULE_NAME "TestModule"

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
    Response()
    {
        mModuleDetails.setName(MODULE_NAME);
        mModuleDetails.setExecutableName("testApplication");
    }
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *data, size_t length)
    {
        auto text = std::make_unique<UMPS::MessageFormats::Text> ();
        text->setContents("Test result"); 
        return text;
    }
    ModuleDetails mModuleDetails;
};

void proxy()
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Debug);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);

    RemoteProxyOptions options;
    options.setFrontendAddress(FRONTEND);
    options.setBackendAddress(BACKEND);
    RemoteProxy proxy(loggerPtr);
    EXPECT_NO_THROW(proxy.initialize(options));
    proxy.start();
    std::this_thread::sleep_for(std::chrono::seconds {3});
    proxy.stop();
}

void replier()
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Debug);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
         = std::make_shared<UMPS::Logging::StdOut> (logger);

    Response response;
    RemoteReplierOptions options;
    options.setAddress(BACKEND);
    options.setCallback(std::bind(&Response::callback,
                                  &response,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
    // Give proxy a chance to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds {10});
    RemoteReplier replier(loggerPtr);
    EXPECT_NO_THROW(replier.initialize(options));
    replier.start();

    std::this_thread::sleep_for(std::chrono::seconds {3}); 
    replier.stop();
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
/*
    auto proxyThread = std::thread(proxy); // Intermediary
    auto replierThread = std::thread(replier); // Setup person on end
    auto requestorThread = std::thread(requestor); // Ask last
     
    requestorThread.join();
    replierThread.join();
    proxyThread.join();
*/
}

}
