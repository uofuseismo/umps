#include <thread>
#include "xpubxsub.hpp"

int main(int argc, char *argv[])
{
    // Create a proxy thread
    auto proxyThread = std::thread(proxy);
    // Give proxy a moment to get up and running
    std::this_thread::sleep_for(std::chrono::milliseconds {500});

    // Subscribers connect first - so as not to miss messages
    auto subscriberThread1 = std::thread(subscriber, 1);
    auto subscriberThread2 = std::thread(subscriber, 2);
    auto subscriberThread3 = std::thread(subscriber, 3); 
    auto subscriberThread4 = std::thread(subscriber, 4);

    // Publisher connects and lets it rip
    auto publisherThread1 = std::thread(publisher, 1);
    auto publisherThread2 = std::thread(publisher, 2);
    auto publisherThread3 = std::thread(publisher, 3);

    // Wait for threads to finish
    proxyThread.join();
    subscriberThread1.join();
    subscriberThread2.join();
    subscriberThread3.join();
    subscriberThread4.join();
    publisherThread1.join();
    publisherThread2.join();
    publisherThread3.join();
    return EXIT_SUCCESS;
}
