#include <pybind11/chrono.h>
#include <umps/services/connectionInformation/availableConnectionsResponse.hpp>
#include <umps/services/connectionInformation/requestor.hpp>
#include <umps/services/connectionInformation/requestorOptions.hpp>
#include <umps/services/connectionInformation/details.hpp>
#include <umps/authentication/zapOptions.hpp>
#include <umps/messaging/requestRouter/requestOptions.hpp>
#include "python/services.hpp"
#include "python/messaging.hpp"
#include "python/logging.hpp"

using namespace UMPS::Python::Services::ConnectionInformation;



AvailableConnectionsResponse::AvailableConnectionsResponse() :
    pImpl(std::make_unique<UMPS::Services::ConnectionInformation::AvailableConnectionsResponse> ())
{
}

AvailableConnectionsResponse::AvailableConnectionsResponse(
    const AvailableConnectionsResponse &response)
{
    *this = response;
}

AvailableConnectionsResponse::AvailableConnectionsResponse(
    AvailableConnectionsResponse &&response) noexcept
{
    *this = std::move(response);
}

AvailableConnectionsResponse&
AvailableConnectionsResponse::operator=(
    const AvailableConnectionsResponse &response)
{
    if (&response == this){return *this;}
    pImpl = std::make_unique<UMPS::Services::ConnectionInformation::AvailableConnectionsResponse> (*response.pImpl);
    return *this;
}

AvailableConnectionsResponse&
AvailableConnectionsResponse::operator=(
    AvailableConnectionsResponse &&response) noexcept
{
    if (&response == this){return *this;}
    pImpl = std::move(response.pImpl);
    return *this;
}

void AvailableConnectionsResponse::setReturnCode(
    const UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode returnCode) noexcept
{
    pImpl->setReturnCode(returnCode);
}

UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode
AvailableConnectionsResponse::getReturnCode() const noexcept
{
    return pImpl->getReturnCode();
}

const UMPS::Services::ConnectionInformation::AvailableConnectionsResponse
&AvailableConnectionsResponse::getNativeClassReference() const
{
    return *pImpl;
}

AvailableConnectionsResponse::~AvailableConnectionsResponse() = default;
///--------------------------------------------------------------------------///

RequestorOptions::RequestorOptions() :
    pImpl(std::make_unique<UMPS::Services::ConnectionInformation::RequestorOptions> ())
{
}

RequestorOptions::RequestorOptions(const RequestorOptions &options)
{
    *this = options;
}

RequestorOptions::RequestorOptions(const UMPS::Services::ConnectionInformation::RequestorOptions &options)
{
    *this = options;
}

RequestorOptions::RequestorOptions(RequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

RequestorOptions& RequestorOptions::operator=(RequestorOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

RequestorOptions& RequestorOptions::operator=(const RequestorOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<UMPS::Services::ConnectionInformation::RequestorOptions> (*options.pImpl);
    return *this;
}

RequestorOptions& RequestorOptions::operator=(
    const UMPS::Services::ConnectionInformation::RequestorOptions &options)
{
    pImpl = std::make_unique<UMPS::Services::ConnectionInformation::RequestorOptions> (options);
    return *this;
}

RequestorOptions::~RequestorOptions() = default;

const UMPS::Services::ConnectionInformation::RequestorOptions 
&RequestorOptions::getNativeClassReference() const
{
    return *pImpl;
}

void RequestorOptions::setAddress(const std::string &address)
{
    pImpl->setAddress(address);
}

std::string RequestorOptions::getAddress() const
{
    auto options = pImpl->getRequestOptions();
    return options.getAddress();
}

void RequestorOptions::setZAPOptions(
    const UMPS::Python::Authentication::ZAPOptions &options)
{
    pImpl->setZAPOptions(options.getNativeClassReference());
}

UMPS::Python::Authentication::ZAPOptions RequestorOptions::getZAPOptions() const
{
    auto options = pImpl->getRequestOptions();
    return UMPS::Python::Authentication::ZAPOptions {options.getZAPOptions()};
}

void RequestorOptions::setTimeOut(const std::chrono::milliseconds &timeOut)
{
    pImpl->setTimeOut(timeOut);
}

std::chrono::milliseconds RequestorOptions::getTimeOut() const
{
    return pImpl->getRequestOptions().getTimeOut();
}

///--------------------------------------------------------------------------///
/*
Requestor::Requestor() :
    pImpl(std::make_unique<UMPS::Services::ConnectionInformation::Requestor> ())
{
}

void Requestor::initialize(const RequestorOptions &options)
{
    pImpl->initialize(options.getNativeClassReference());
}

bool Requestor::isInitialized() const noexcept
{
    return pImpl->isInitialized();
}

Requestor::Requestor(
    UMPS::Python::Messaging::Context &context)
{
    auto umpsContext = context.getSharedPointer();
    pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Requestor> (umpsContext);
}

Requestor::Requestor(
    UMPS::Python::Logging::ILog &logger)
{
    auto umpsLogger = logger.getSharedPointer();
    pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Requestor> (umpsLogger);
}

Requestor::Requestor(
    UMPS::Python::Messaging::Context &context,
    UMPS::Python::Logging::ILog &logger)
{
    auto umpsContext = context.getSharedPointer();
    auto umpsLogger = logger.getSharedPointer();
    pImpl = std::make_unique<UMPS::Services::ConnectionInformation::Requestor> (umpsContext, umpsLogger);
}

void Requestor::disconnect()
{
    pImpl->disconnect();
}

Requestor::~Requestor() = default;
*/
