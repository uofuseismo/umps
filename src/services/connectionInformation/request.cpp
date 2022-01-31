#include <vector>
#include <zmq.hpp>
#include "umps/services/connectionInformation/request.hpp"
#include "umps/services/connectionInformation/requestOptions.hpp"
#include "umps/services/connectionInformation/details.hpp"
#include "umps/services/connectionInformation/availableConnectionsRequest.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/messaging/requestRouter/request.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

namespace URequestRouter = UMPS::Messaging::RequestRouter;
using namespace UMPS::Services::ConnectionInformation;

class Request::RequestImpl
{
public:
    /// Constructors
    RequestImpl() = delete;
    RequestImpl(std::shared_ptr<zmq::context_t> context,
                std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
        else
        {
            mLogger = logger;
        }
        mRequestor = std::make_unique<URequestRouter::Request>
                     (context, mLogger);
    }
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::unique_ptr<URequestRouter::Request> mRequestor;
    RequestOptions mOptions;
};

/// C'tor
Request::Request() :
    pImpl(std::make_unique<RequestImpl> (nullptr, nullptr))
{
}

/// Move c'tor
Request::Request(Request &&request) noexcept
{
    *this = std::move(request);
}

/// Destructor
Request::~Request() = default;

/// Move assignment
Request& Request::operator=(Request &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Initialize
void Request::initialize(const RequestOptions &requestOptions)
{
    auto options = requestOptions.getRequestOptions();
    if (!options.haveEndPoint())
    {
        throw std::invalid_argument("End point not set");
    } 
    pImpl->mRequestor->initialize(options);
}

/// Initialized?
bool Request::isInitialized() const noexcept
{
    return pImpl->mRequestor->isInitialized();
} 

/// Get all connection details
std::vector<Details> Request::getDetails() const
{
    if (!isInitialized())
    {
        throw std::runtime_error("Request client not initialized");
    }
    std::vector<Details> result;
    AvailableConnectionsRequest requestMessage;
    auto message = pImpl->mRequestor->request(requestMessage);
    if (message == nullptr)
    {
        auto detailsMessage
            = static_unique_pointer_cast<AvailableConnectionsResponse>
              (std::move(message));
    //    return detailsMessage->getDetails();
 
    }
    else
    {
        pImpl->mLogger->warn("Request timed out");
    }
    return result;
}

/// Get proxyBroadcast frontend 

/// Disconnect
void Request::disconnect()
{
    pImpl->mRequestor->disconnect();
}
