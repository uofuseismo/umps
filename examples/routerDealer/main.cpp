#include <iostream>
#include <thread>
#include <array>
#include "routerDealer.hpp"

int main()
{
    // Create a proxy thread
    auto proxyThread = std::thread(proxy);
    // Give proxy a moment to get up and running
    std::this_thread::sleep_for(std::chrono::milliseconds {500});

    // Make the servers
    std::array<std::thread, N_SERVERS> serverThreads;
    for (int i = 0; i < N_SERVERS; ++i)
    {
        serverThreads.at(i) = std::thread(server, i + 1);
    }

    // Give servers a chance to connect
    std::this_thread::sleep_for(std::chrono::milliseconds {500});

    // Make the clients
    std::array<std::thread, N_CLIENTS> clientThreads;
    for (int i = 0; i < N_CLIENTS; ++i)
    {
        clientThreads.at(i) = std::thread(client, i + 1);
    }

    // Wait for threads to finish
    proxyThread.join();
    for (auto &thread : clientThreads)
    {
        if (thread.joinable()){thread.join();}
    }
    for (auto &thread : serverThreads)
    {
        if (thread.joinable()){thread.join();}
    }
    return EXIT_SUCCESS;
}
