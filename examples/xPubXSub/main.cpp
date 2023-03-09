#include <iostream>
#include <thread>
#include "xpubxsub.hpp"

int main(int argc, char *argv[])
{
    // Create a proxy thread
    auto proxyThread = std::thread(proxy);

    // Subscribers connect first - so as not to miss messages
    std::this_thread::sleep_for(std::chrono::milliseconds {500});
    auto subscriberThread1 = std::thread(subscriber, 1);
    auto subscriberThread2 = std::thread(subscriber, 2);

    // Publisher connects and lets it rip
    std::this_thread::sleep_for(std::chrono::milliseconds {1000});
    auto publisherThread = std::thread(publisher);

    // Wait for threads to finish
    proxyThread.join();
    subscriberThread1.join();
    subscriberThread2.join();
    publisherThread.join();
    return EXIT_SUCCESS;
}
