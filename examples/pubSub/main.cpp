#include <thread>
#include "pubsub.hpp"

int main(int argc, char *argv[])
{
    // Actually let the subscribers `connect' first
    auto subscriberThread1 = std::thread(subscriber, 1);
    auto subscriberThread2 = std::thread(subscriber, 2);
    auto subscriberThread3 = std::thread(subscriber, 3);
    // Now create the publisher
    auto publisherThread = std::thread(publisher);
    // Wait for threads to finish
    subscriberThread1.join();
    subscriberThread2.join();
    subscriberThread3.join();
    publisherThread.join();
    return EXIT_SUCCESS;
}
