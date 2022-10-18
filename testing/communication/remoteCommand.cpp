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
#include "umps/services/command/terminateRequest.hpp"
#include "umps/services/command/terminateResponse.hpp"
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

class ResponderProcess
{
public:
    explicit ResponderProcess(std::shared_ptr<UMPS::Logging::ILog> logger,
                              const int id) :
        mLogger(logger),
        mIdentifier(id)
    {
        ModuleDetails details;
        mName = "test_module_" + std::to_string(id);
        details.setName(mName);
        mReplier = std::make_shared<Replier> (mLogger);

        ReplierOptions options;
        mOptions.setModuleDetails(details);
        mOptions.setAddress(BACKEND);
        mOptions.setCallback(std::bind(&ResponderProcess::callback,
                                       this,
                                       std::placeholders::_1,
                                       std::placeholders::_2,
                                       std::placeholders::_3));
        //setStopCallback(mStopCallback); 
    }
    ~ResponderProcess(){stop();}
    [[nodiscard]] std::string getName() const noexcept //override
    {
        return mName;
    }
    void start()
    {
        mReplier->initialize(mOptions);
        if (mReplier){mReplier->start();}
    }
    void stop()
    {
        if (mReplier && isRunning())
        {
            mReplier->stop();
        }
    }
    [[nodiscard]] bool isRunning() const noexcept
    {
        return mReplier->isRunning();
    }
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage>
        callback(const std::string &messageType,
                 const void *data, size_t length)
    {
        UMPS::Services::Command::AvailableCommandsRequest
            availableCommandsRequest;
        UMPS::Services::Command::TerminateRequest terminateRequest;
        if (messageType == availableCommandsRequest.getMessageType())
        {
            UMPS::Services::Command::AvailableCommandsResponse response;
            response.setCommands("test_module_" + std::to_string(mIdentifier));
            // Allows a timeout to happen
            //std::this_thread::sleep_for(std::chrono::milliseconds {200});
            return response.clone();
        }
        else if (messageType == terminateRequest.getMessageType())
        {
            issueStopNotification();
            UMPS::Services::Command::TerminateResponse terminateResponse;
            terminateResponse.setReturnCode(
               UMPS::Services::Command::TerminateResponse::ReturnCode::Success);
            return terminateResponse.clone();
        }
        auto text = std::make_unique<UMPS::MessageFormats::Text> (); 
        text->setContents("Test result"); 
        return text;
    }
    /// Have this thread tell the main thread to stop the proxy.
    void issueStopNotification()
    {
        mLogger->debug("Issuing stop notification...");
        {
            std::lock_guard<std::mutex> lock(mStopContext);
            mStopRequested = true;
        }
        mStopCondition.notify_one();
    }
    /// Have the main thread either react to a stop notification or,
    /// if none happens, shut down the module after a certain amount
    /// of time
    void handleMainThread(const std::chrono::milliseconds &maxTime)
    {
        auto t0 = std::chrono::high_resolution_clock::now();
        while (!mStopRequested)
        {
            std::unique_lock<std::mutex> lock(mStopContext);
            mStopCondition.wait_for(lock,
                                    std::chrono::milliseconds {100},
                                    [this]
                                    {
                                          return mStopRequested;
                                    });
            lock.unlock();
            auto now = std::chrono::high_resolution_clock::now();
            if (now - t0 > maxTime){break;}
        }
        if (mStopRequested)
        {
            mLogger->debug("Stop request received.  Terminating...");
        }
        else
        {
            mLogger->debug("Main thread time limit hit.  Terminating...");
        }
        stop();
    }
///private:
    mutable std::mutex mStopContext;
    std::function<void ()> mStopCallback{[this]{issueStopNotification();}};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::shared_ptr<Replier> mReplier{nullptr};
    std::condition_variable mStopCondition;
    ReplierOptions mOptions;
    std::string mName;
    int mIdentifier{0};
    bool mStopRequested{false};
};

void proxy()
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Info);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    const std::vector<std::chrono::milliseconds> pingIntervals
    {
        std::chrono::milliseconds {10},
        std::chrono::milliseconds {20}
    };

    ProxyOptions options;
    options.setFrontendAddress(FRONTEND);
    options.setBackendAddress(BACKEND);
    options.setPingIntervals(pingIntervals);
    Proxy proxy(loggerPtr);
    EXPECT_NO_THROW(proxy.initialize(options));
    EXPECT_NO_THROW(proxy.start());
    std::this_thread::sleep_for(std::chrono::seconds {2});
    proxy.stop();
}

void replier(int id)
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::Info);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
         = std::make_shared<UMPS::Logging::StdOut> (logger);
    //std::unique_ptr<UMPS::Modules::IProcess> responder
    auto responder = std::make_unique<ResponderProcess> (loggerPtr, id);
    responder->start();
    responder->handleMainThread(std::chrono::seconds {3});
/*
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
*/
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
        // Request commands for this module
        auto commands = requestor.getCommands(m.getName());
        // Ensure these commands make sense
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

    // Let workers get ready
    std::this_thread::sleep_for(std::chrono::milliseconds {500});
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
