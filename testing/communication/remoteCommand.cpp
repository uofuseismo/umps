#include <thread>
#include <chrono>
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/proxyServices/command/availableModulesResponse.hpp"
#include "umps/proxyServices/command/proxy.hpp"
#include "umps/proxyServices/command/proxyOptions.hpp"
#include "umps/proxyServices/command/requestor.hpp"
#include "umps/proxyServices/command/requestorOptions.hpp"
#include "umps/proxyServices/command/replier.hpp"
#include "umps/proxyServices/command/replierOptions.hpp"
#include "umps/proxyServices/command/moduleDetails.hpp"
#include "umps/messaging/context.hpp"
#include "umps/messageFormats/text.hpp"
#include "umps/logging/stdout.hpp"
#include <gtest/gtest.h>

#define FRONTEND "tcp://127.0.0.1:5000"
#define BACKEND  "tcp://127.0.0.1:5001"
#define MODULE_NAME "TestModule"

namespace
{

using namespace UMPS::ProxyServices::Command;

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
    explicit Response(const int id) :
        mIdentifier(id)
    {
        mModuleDetails.setName(MODULE_NAME);
        mModuleDetails.setExecutableName("testApplication");
        
    }
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *data, size_t length)
    {
        UMPS::Services::Command::AvailableCommandsRequest
            availableCommandsRequest;
        if (messageType == availableCommandsRequest.getMessageType())
        {
            UMPS::Services::Command::AvailableCommandsResponse response;
            response.setCommands("test_module_" + std::to_string(mIdentifier));
            return response.clone();
        }
        auto text = std::make_unique<UMPS::MessageFormats::Text> ();
        text->setContents("Test result"); 
        return text;
    }
    ModuleDetails mModuleDetails;
    int mIdentifier{0};
};

void proxy()
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Info);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);

    ProxyOptions options;
    options.setFrontendAddress(FRONTEND);
    options.setBackendAddress(BACKEND);
    Proxy proxy(loggerPtr);
    EXPECT_NO_THROW(proxy.initialize(options));
    EXPECT_NO_THROW(proxy.start());
    std::this_thread::sleep_for(std::chrono::seconds {3});
    proxy.stop();
}

void replier(int id)
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Info);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
         = std::make_shared<UMPS::Logging::StdOut> (logger);
    ModuleDetails details;
    details.setName("test_module_" + std::to_string(id));
 
    Response response(id);
    ReplierOptions options;
    options.setModuleDetails(details);
    options.setAddress(BACKEND);
    options.setCallback(std::bind(&Response::callback,
                                  &response,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
    Replier replier(loggerPtr);
    // Give proxy a chance to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds {10});
    EXPECT_NO_THROW(replier.initialize(options));
    EXPECT_NO_THROW(replier.start());

    std::this_thread::sleep_for(std::chrono::seconds {2}); 
    replier.stop();
}

void requestor()
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Info);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
         = std::make_shared<UMPS::Logging::StdOut> (logger);

    Requestor requestor(loggerPtr);
    RequestorOptions options;
    options.setAddress(FRONTEND);
    // Give proxy and respondor a chance to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds {20});
    requestor.initialize(options);
    auto modules = requestor.getAvailableModules();
    // Send an invalid request
    EXPECT_THROW(auto temp = requestor.getCommands("bogus"), std::exception);
    // Now get the modules
    for (const auto &m : modules->getModules())
    {
        auto commands = requestor.getCommands(m.getName());
        auto command = commands->getCommands();
        EXPECT_EQ(m.getName(), command);
    }
}

TEST(ProxyServicesCommand, Command)
{
    auto proxyThread = std::thread(proxy); // Intermediary
    auto replierThread1 = std::thread(replier, 1);
//    auto replierThread2 = std::thread(replier, 2);
//    auto replierThread3 = std::thread(replier, 3);
    auto requestorThread1 = std::thread(requestor); // Ask last
//    auto requestorThread2 = std::thread(requestor);
     
    requestorThread1.join();
//    requestorThread2.join();
    replierThread1.join();
//    replierThread2.join();
//    replierThread3.join();
    proxyThread.join();
}

}
