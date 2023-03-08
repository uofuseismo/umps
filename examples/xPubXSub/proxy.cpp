#include <thread>
#include <chrono>
#include <umps/messaging/xPublisherXSubscriber/proxy.hpp>
#include <umps/messaging/xPublisherXSubscriber/proxyOptions.hpp>

using namespace UMPS::Messaging::XPublisherXSubscriber;

void proxy()
{
    ProxyOptions proxyOptions;
    // Data flows from frontend to backend
    proxyOptions.setFrontendAddress("tcp://127.0.0.1:5555");
    proxyOptions.setBackendAddress("tcp://127.0.0.1:5556");

    // Create the proxy
    Proxy proxy;
    proxy.initialize(proxyOptions);

/*
    // Run the proxy in a separate thread
    auto proxyThread = std::thread(&proxy, &Proxy::start);
*/ 
    // Main thread sleeps a bit
    std::this_thread::sleep_for(std::chrono::seconds {2});
    // All done - main thread tells the proxy to shut down
    proxy.stop();
//    proxyThread.join();
}
