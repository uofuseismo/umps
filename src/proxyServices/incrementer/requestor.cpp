#include <iostream>
#include <string>
#ifndef NDEBUG
#include <cassert>
#endif
#include "umps/proxyServices/incrementer/requestor.hpp"
#include "umps/proxyServices/incrementer/requestorOptions.hpp"
#include "umps/proxyServices/incrementer/itemsRequest.hpp"
#include "umps/proxyServices/incrementer/itemsResponse.hpp"
#include "umps/proxyServices/incrementer/incrementRequest.hpp"
#include "umps/proxyServices/incrementer/incrementResponse.hpp"
#include "umps/messaging/routerDealer/requestOptions.hpp"
#include "umps/messaging/routerDealer/request.hpp"
#include "umps/services/connectionInformation/socketDetails/request.hpp"
#include "private/staticUniquePointerCast.hpp"

using namespace UMPS::ProxyServices::Incrementer;
namespace UCI = UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace URouterDealer = UMPS::Messaging::RouterDealer;

class Requestor::RequestorImpl
{
public:
    RequestorImpl(std::shared_ptr<UMPS::Messaging::Context> context,
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

Requestor::Requestor(std::shared_ptr<UMPS::Messaging::Context> &context) :
    pImpl(std::make_unique<RequestorImpl> (context, nullptr))
{
}

Requestor::Requestor(std::shared_ptr<UMPS::Messaging::Context> &context,
                     std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<RequestorImpl> (context, logger))
{
}

/// Connection information
UCI::SocketDetails::Request Requestor::getSocketDetails() const
{
    return pImpl->mRequestor->getSocketDetails();
}

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
std::unique_ptr<ItemsResponse> Requestor::request(const ItemsRequest &request)
{
    auto response 
        = static_unique_pointer_cast<ItemsResponse>
          (pImpl->mRequestor->request(request));
    return response;
}

/// Request data
std::unique_ptr<IncrementResponse>
    Requestor::request(const IncrementRequest &request)
{
    if (!request.haveItem()){throw std::invalid_argument("Item not set");}
    auto response 
        = static_unique_pointer_cast<IncrementResponse>
          (pImpl->mRequestor->request(request));
    return response;
}
