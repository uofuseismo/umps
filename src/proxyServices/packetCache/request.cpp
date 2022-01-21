#include <string>
#include <zmq.hpp>
#include "umps/proxyServices/packetCache/request.hpp"
#include "umps/proxyServices/packetCache/requestOptions.hpp"
#include "umps/proxyServices/packetCache/sensorRequest.hpp"
#include "umps/proxyServices/packetCache/sensorResponse.hpp"
#include "umps/proxyServices/packetCache/dataRequest.hpp"
#include "umps/proxyServices/packetCache/dataResponse.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace URouterDealer = UMPS::Messaging::RouterDealer;

class Request::RequestImpl
{
public:
    RequestImpl(std::shared_ptr<zmq::context_t> context,
                std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        mRequestor = std::make_unique<URouterDealer::Request> (context, logger);
    }
    std::unique_ptr<URouterDealer::Request> mRequestor;
    RequestOptions mRequestOptions;
};

/// C'tor
Request::Request() :
    pImpl(std::make_unique<RequestImpl> (nullptr, nullptr))
{
}

Request::Request(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (nullptr, logger))
{
}

/// Connection information
UCI::SocketDetails::Request Request::getSocketDetails() const
{
    return pImpl->mRequestor->getSocketDetails();
}

/*
Request::Request(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<RequestImpl> (context, nullptr))
{
}

Request::Request(std::shared_ptr<zmq::context_t> &context,
                 std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestImpl> (context, logger))
{
}
*/

/// Move c'tor
Request::Request(Request &&request) noexcept
{
    *this = std::move(request);
}

/// Move assignment
Request& Request::operator=(Request &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
Request::~Request() = default;

/// Initialize
void Request::initialize(const RequestOptions &options)
{
    if (!options.haveEndPoint())
    {
        throw std::invalid_argument("End point not set");
    }
    pImpl->mRequestOptions = options;
    pImpl->mRequestor->initialize(pImpl->mRequestOptions.getRequestOptions());
}

/// Initialized?
bool Request::isInitialized() const noexcept
{
    return pImpl->mRequestor->isInitialized();
}


/// Disconnect
void Request::disconnect()
{
    pImpl->mRequestor->disconnect();
}

/// Request sensor information
std::unique_ptr<SensorResponse> Request::request(const SensorRequest &request)
{
    auto response 
        = static_unique_pointer_cast<SensorResponse>
          (pImpl->mRequestor->request(request));
    return response;
}

/// Request data
/*
std::unique_ptr<DataResponse> Request::request(const DataResponse &request)
{
    auto response 
        = static_unique_pointer_cast<DataResponse>
          (pImpl->mRequestor->request(request));
    return response;
}
*/
