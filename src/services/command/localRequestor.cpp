#include <string>
#include <filesystem>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/services/command/localRequestor.hpp"
#include "umps/services/command/localRequestorOptions.hpp"
#include "umps/services/command/availableCommandsRequest.hpp"
#include "umps/services/command/availableCommandsResponse.hpp"
#include "umps/services/command/commandRequest.hpp"
#include "umps/services/command/commandResponse.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/context.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::Services::Command;

class LocalRequestor::LocalRequestorImpl
{
public:
    /// @brief Constructor
    LocalRequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
                       std::shared_ptr<UMPS::Logging::ILog> logger) :
        mRequest(std::make_unique<UMPS::Messaging::RequestRouter::Request> (context, logger))
    {
/*
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
*/
        // Make the message types
        std::unique_ptr<UMPS::MessageFormats::IMessage>
            availableCommandsResponseMessage
               = std::make_unique<AvailableCommandsResponse> ();
        std::unique_ptr<UMPS::MessageFormats::IMessage> commandsResponse
            = std::make_unique<CommandResponse> ();
        mMessageFormats.add(availableCommandsResponseMessage);
        mMessageFormats.add(commandsResponse);
    }
/*
    /// @brief Initialized?
    [[nodiscard]] bool isInitialized() const noexcept
    {
        return mConnected;
    }
    /// @brief Makes a request
    std::unique_ptr<UMPS::MessageFormats::IMessage>
        request(const UMPS::MessageFormats::IMessage &request)
    {
        if (!isInitialized()){throw std::runtime_error("Not initialized");}
        auto messageType = request.getMessageType();
        if (messageType.empty())
        {
            mLogger->error("Message type is empty");
        }
        auto requestMessage = request.toMessage();
        if (requestMessage.empty())
        {
            mLogger->error("Message contents is empty");
        }
        // Send the message
        mLogger->debug("Sending message: " + messageType);
        zmq::const_buffer headerRequest{messageType.data(), messageType.size()};

        mRequest->send(headerRequest, zmq::send_flags::sndmore);
        zmq::const_buffer bufferRequest{requestMessage.data(),
                                        requestMessage.size()};
        mRequest->send(bufferRequest);
        // Wait for the response
        mLogger->debug("Blocking for response...");
        // Receive all parts of the message
        zmq::multipart_t responseReceived(*mRequest);
        if (responseReceived.empty()){return nullptr;} // Timeout
#ifndef NDEBUG
        assert(responseReceived.size() == 2); 
#else
        if (responseReceived.size() != 2)
        {
            mLogger->error("Only 2-part messages handled");
            throw std::runtime_error("Only 2-part messages handled");
        }   
#endif
        // Unpack the response
        std::string responseMessageType = responseReceived.at(0).to_string();
        if (!mMessageFormats.contains(responseMessageType))
        {
            throw std::runtime_error("Unhandled response type: "
                                   + responseMessageType);
        }
        const auto payload
             = static_cast<char *> (responseReceived.at(1).data());
        auto responseLength = responseReceived.at(1).size();
        auto response = mMessageFormats.get(responseMessageType);
        try
        {
            response->fromMessage(payload, responseLength);
        }
        catch (const std::exception &e)
        {
            auto errorMsg = "Failed to unpack message of type: " + messageType;
            mLogger->error(errorMsg);
            throw;
        }
        return response;
    }
*/
    /// @brief Disconnect
    void disconnect()
    {
        mRequest->disconnect();
/*
        if (mConnected){mRequest->disconnect(mIPCName);}
        mConnected = false;
*/
    }
    UMPS::MessageFormats::Messages mMessageFormats;
    std::unique_ptr<UMPS::Messaging::RequestRouter::Request> mRequest{nullptr};
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    LocalRequestorOptions mLocalRequestorOptions;
    UMPS::Messaging::RequestRouter::RequestOptions mRequestOptions;
    std::filesystem::path mIPCFileName;
    std::string mIPCName; // e.g., ipc://mIPCFileName
    LocalRequestorOptions mOptions;
};

/// C'tor
LocalRequestor::LocalRequestor() :
    pImpl(std::make_unique<LocalRequestorImpl> (nullptr, nullptr))
{
}

LocalRequestor::LocalRequestor(
    std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<LocalRequestorImpl> (context, nullptr))
{
}

LocalRequestor::LocalRequestor(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<LocalRequestorImpl> (nullptr, logger))
{
}

LocalRequestor::LocalRequestor(
    std::shared_ptr<UMPS::Messaging::Context> &context,
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<LocalRequestorImpl> (context, logger))
{
}

/// Move c'tor
LocalRequestor::LocalRequestor(LocalRequestor &&requestor) noexcept
{
    *this = std::move(requestor);
}

/// Move assignment
LocalRequestor& LocalRequestor::operator=(LocalRequestor &&requestor) noexcept
{
    if (&requestor == this){return *this;}
    pImpl = std::move(requestor.pImpl);
    return *this;
}

/// Destructor
LocalRequestor::~LocalRequestor() = default;

/// Initialize
void LocalRequestor::initialize(const LocalRequestorOptions &options)
{
    if (!options.haveModuleName())
    {
        throw std::invalid_argument("Module name not set");
    }
    pImpl->disconnect();
    pImpl->mIPCFileName = options.getIPCFileName();
    if (!std::filesystem::exists(pImpl->mIPCFileName))
    {
        throw std::runtime_error("IPC file " + pImpl->mIPCFileName.string()
                               + " does not exist");
    }
    UMPS::Messaging::RequestRouter::RequestOptions requestOptions;
    pImpl->mIPCName = "ipc://" + pImpl->mIPCFileName.string();
    requestOptions.setAddress(pImpl->mIPCName);
    requestOptions.setTimeOut(options.getReceiveTimeOut());

    auto messageFormats = pImpl->mMessageFormats.get();
    for (auto &messageFormat : messageFormats)
    {
        requestOptions.addMessageFormat(messageFormat.second);
    }
    pImpl->mRequest->initialize(requestOptions);
    pImpl->mLocalRequestorOptions = options;
    pImpl->mRequestOptions = requestOptions;
}

/*
    auto timeOut = options.getReceiveTimeOut();
    pImpl->mLogger->debug("Connecting to: " + pImpl->mIPCName);
    pImpl->mRequest->set(zmq::sockopt::rcvtimeo,
                         static_cast<int> (timeOut.count()));
    pImpl->mRequest->connect(pImpl->mIPCName);
    pImpl->mIPCName = pImpl->mRequest->get(zmq::sockopt::last_endpoint);
    pImpl->mConnected = true;
}
*/

/// Initialized?
bool LocalRequestor::isInitialized() const noexcept
{
    return pImpl->mRequest->isInitialized();
}

/// Commands
std::unique_ptr<AvailableCommandsResponse> LocalRequestor::getCommands() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Requestor not initialized");
    }
    std::unique_ptr<AvailableCommandsResponse> result{nullptr};
    AvailableCommandsRequest requestMessage;
    auto message = pImpl->mRequest->request(requestMessage);
    if (message != nullptr)
    {
        result = static_unique_pointer_cast<AvailableCommandsResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

std::unique_ptr<CommandResponse> LocalRequestor::issueCommand(
    const CommandRequest &request)
{
    if (!isInitialized())
    {   
        throw std::runtime_error("Requestor not initialized");
    }   
    std::unique_ptr<CommandResponse> result{nullptr};
    auto message = pImpl->mRequest->request(request);
    if (message != nullptr)
    {
        result = static_unique_pointer_cast<CommandResponse>
                 (std::move(message));
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;

}

void LocalRequestor::disconnect()
{
    pImpl->disconnect();
}
