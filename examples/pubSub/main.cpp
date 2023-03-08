#include <iostream>
#include <thread>
#include "pubsub.hpp"

int main(int argc, char *argv[])
{
    // Actually let the subscriber `connect' first
    auto subscriberThread = std::thread(subscriber);
    // Now create the publisher
    auto publisherThread = std::thread(publisher);
    // Wait for threads to finish
    subscriberThread.join();
    publisherThread.join();
    return EXIT_SUCCESS;
}
