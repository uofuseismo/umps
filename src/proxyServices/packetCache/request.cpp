#include <iostream>
#include <string>
#include <zmq.hpp>
#include "umps/proxyServices/packetCache/requestor.hpp"
#include "umps/proxyServices/packetCache/requestorOptions.hpp"
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

class Requestor::RequestorImpl
{
public:
    RequestorImpl(std::shared_ptr<zmq::context_t> context,
                  std::shared_ptr<UMPS::Logging::ILog> logger)
    {
        mRequestor = std::make_unique<URouterDealer::Request> (context, logger);
    }
    std::unique_ptr<URouterDealer::Request> mRequestor;
    RequestorOptions mRequestOptions;
};

/// C'tor
Requestor::Requestor() :
    pImpl(std::make_unique<RequestorImpl> (nullptr, nullptr))
{
}

Requestor::Requestor(std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestorImpl> (nullptr, logger))
{
}

/// Connection information
UCI::SocketDetails::Request Requestor::getSocketDetails() const
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
Requestor::Requestor(Requestor &&request) noexcept
{
    *this = std::move(request);
}

/// Move assignment
Requestor& Requestor::operator=(Requestor &&request) noexcept
{
    if (&request == this){return *this;}
    pImpl = std::move(request.pImpl);
    return *this;
}

/// Destructor
Requestor::~Requestor() = default;

/// Initialize
void Requestor::initialize(const RequestorOptions &options)
{
    if (!options.haveAddress())
    {
        throw std::invalid_argument("Address not set");
    }
    pImpl->mRequestOptions = options;
    pImpl->mRequestor->initialize(pImpl->mRequestOptions.getRequestOptions());
}

/// Initialized?
bool Requestor::isInitialized() const noexcept
{
    return pImpl->mRequestor->isInitialized();
}


/// Disconnect
void Requestor::disconnect()
{
    pImpl->mRequestor->disconnect();
}

/// Request sensor information
std::unique_ptr<SensorResponse> Requestor::request(const SensorRequest &request)
{
    auto response 
        = static_unique_pointer_cast<SensorResponse>
          (pImpl->mRequestor->request(request));
    return response;
}

/// Request data
//template<>
std::unique_ptr<DataResponse<double>> 
    Requestor::request(const DataRequest &request)
{
    if (!request.haveNetwork()){throw std::invalid_argument("Network not set");}
    if (!request.haveStation()){throw std::invalid_argument("Station not set");}
    if (!request.haveChannel()){throw std::invalid_argument("Channel not set");}
    if (!request.haveLocationCode())
    {
        throw std::invalid_argument("Location code not set");
    }
    auto response 
        = static_unique_pointer_cast<DataResponse<double>>
          (pImpl->mRequestor->request(request));
    return response;
}
