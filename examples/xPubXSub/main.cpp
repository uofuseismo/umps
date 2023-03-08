#include <iostream>
#include <thread>
#include "xpubxsub.hpp"

int main(int argc, char *argv[])
{
    // Create a proxy thread
    auto proxyThread = std::thread(proxy);
    std::this_thread::sleep_for(std::chrono::milliseconds {100});
    // Subscriber connects first - so as not to miss messages
    auto subscriberThread = std::thread(subscriber);
    std::this_thread::sleep_for(std::chrono::milliseconds {100});
    // Connect the publisher and let it rip
    auto publisherThread = std::thread(publisher);
    // Wait for threads to finish
    proxyThread.join();
    subscriberThread.join();
    publisherThread.join();
    return EXIT_SUCCESS;
}
