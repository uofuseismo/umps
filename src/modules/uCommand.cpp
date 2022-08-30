#include <iostream>
#include <string>
#include "umps/logging/stdout.hpp"
#include "umps/services/command/localRequestor.hpp"
#include "umps/services/command/localRequestorOptions.hpp"

#include <chrono>
#include <filesystem>
#include <thread>
#include <zmq.hpp>
#include "umps/messaging/context.hpp"
#include "umps/messageFormats/message.hpp"

class IPCReply
{
public:
    /// @brief Constructor
    IPCReply(std::shared_ptr<UMPS::Messaging::Context> context,
             std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> (1);
        }
        else
        {
            mContext = context;
        }
        // Make the logger
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        else
        {
            mLogger = logger;
        }
        // Now make the socket
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mReply = std::make_unique<zmq::socket_t> (*contextPtr,
                                                  zmq::socket_type::rep);
    } 
    /// @brief Destructor
    ~IPCReply()
    {
        disconnect();
    }
    /// @brief Allows the reply mechanism to bind to an address
    void bind(const std::string &moduleName)
    {
        if (!std::filesystem::exists(mIPCRootDirectory))
        {
            std::string debugMessage{"Creating IPC directory: "};
            debugMessage = debugMessage + mIPCRootDirectory.string();
            mLogger->debug(debugMessage);
            if (!std::filesystem::create_directories(mIPCRootDirectory))
            {
                std::string errorMessage{"Failed to make IPC directory: "};
                errorMessage = errorMessage + mIPCRootDirectory.string();
                mLogger->error(errorMessage);
                throw std::runtime_error(errorMessage);
            }
            // Update the permissions
            std::filesystem::permissions(mIPCRootDirectory,
                                         std::filesystem::perms::owner_read  |
                                         std::filesystem::perms::owner_write |
                                         std::filesystem::perms::owner_exec);
        }
        mIPCFileName = mIPCRootDirectory
                     / std::filesystem::path{moduleName + ".ipc"};
        mIPCName = "ipc://" + mIPCFileName.string();
        disconnect();
        mLogger->debug("Connecting to: " + mIPCName);
        mReply->bind(mIPCName);
        mIPCName = mReply->get(zmq::sockopt::last_endpoint);
        mConnected = true;
    }
    /// @brief Disconnect
    void disconnect()
    {
        if (mConnected)
        {
            mReply->disconnect(mIPCName);
            std::this_thread::sleep_for(std::chrono::milliseconds{100});
            if (std::filesystem::exists(mIPCFileName))
            {
                std::filesystem::remove(mIPCFileName);
            }
        }
        mConnected = false;
    }
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<zmq::socket_t> mReply{nullptr};
    std::filesystem::path mIPCRootDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    std::filesystem::path mIPCFileName;
    std::string mIPCName;
    bool mConnected{false};
};

//----------------------------------------------------------------------------//

class IPCRequest
{
public:
    /// @brief Constructor
    IPCRequest(std::shared_ptr<UMPS::Messaging::Context> context,
               std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (context == nullptr)
        {
            mContext = std::make_shared<UMPS::Messaging::Context> (1);
        }
        else
        {
            mContext = context;
        }
        // Make the logger
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        else
        {
            mLogger = logger;
        }
        // Now make the socket
        auto contextPtr = reinterpret_cast<zmq::context_t *>
                          (mContext->getContext());
        mRequest = std::make_unique<zmq::socket_t> (*contextPtr,
                                                    zmq::socket_type::req);
    }
    /// @brief Destructor
    ~IPCRequest()
    {   
        disconnect();
    }
    /// @brief Disconnect
    void disconnect()
    {
        if (mConnected){mRequest->disconnect(mIPCName);}
        mConnected = false;
    }
    /// @brief Allows the reply mechanism to bind to an address
    void connect(const std::string &moduleName)
    {
        auto fileName = mIPCRootDirectory
                     / std::filesystem::path{moduleName + ".ipc"};
        if (!std::filesystem::exists(fileName))
        {
             throw std::invalid_argument("IPC file " + fileName.string()
                                       + " does not exist");
        }
        mIPCFileName = fileName; 
        mIPCName = "ipc://" + mIPCFileName.string();
        disconnect();
        mLogger->debug("Connecting to: " + mIPCName);
        mRequest->set(zmq::sockopt::rcvtimeo,
                      static_cast<int> (mTimeOut.count()));
        mRequest->connect(mIPCName);
        mIPCName = mRequest->get(zmq::sockopt::last_endpoint);
        mConnected = true;
    }
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    std::unique_ptr<zmq::socket_t> mRequest{nullptr};
    std::filesystem::path mIPCRootDirectory
        = std::filesystem::path{std::string{std::getenv("HOME")}}
        / std::filesystem::path{".local/share/UMPS/ipc"};
    std::filesystem::path mIPCFileName;
    std::string mIPCName;
    std::chrono::milliseconds mTimeOut{100};
    bool mConnected{false};
};

int main(int argc, char *argv[])
{
IPCReply reply(nullptr, nullptr);
IPCRequest request(nullptr, nullptr);
try
{
reply.bind("temp");
request.connect("temp");
}
catch (const std::exception &e)
{
std::cerr << e.what() << std::endl;
return EXIT_FAILURE;
}
    while (true)
    {
        std::string command;
        std::cout << "uCommand$";
        std::cin >> command;
        if (command == "quit")
        {
            break;
        }
        else if (command == "connect")
        {
            // Get the module name
            
        }
        else
        {
            if (command != "help")
            {
                std::cout << "Unhandled command: " << command << std::endl;
            }
            std::cout << "Options:" << std::endl;
            std::cout << "   help                  Prints this message." << std::endl;
            std::cout << "   connect [ModuleName]  Connects to the module." << std::endl;
            std::cout << "   quit                  Exits this application." << std::endl;
        }
    }
    return EXIT_SUCCESS;
}
