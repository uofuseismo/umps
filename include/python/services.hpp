#ifndef UMPS_PYTHON_SERVICES_HPP
#define UMPS_PYTHON_SERVICES_HPP
#include <pybind11/pybind11.h>
#include "messageFormats.hpp"
#include "authentication.hpp"
#include <string>
#include <chrono>
#include <umps/services/connectionInformation/details.hpp>
#include <umps/services/connectionInformation/requestor.hpp>
#include <umps/services/connectionInformation/requestorOptions.hpp>
#include <umps/services/connectionInformation/availableConnectionsRequest.hpp>
#include <umps/services/connectionInformation/availableConnectionsResponse.hpp>
#include <umps/services/connectionInformation/enums.hpp>
namespace UMPS::Python::Logging
{
 class ILog;
}
namespace UMPS::Python::Messaging
{
 class Context;
}
namespace UMPS::Python::Services
{
namespace ConnectionInformation
{
class Details;
///--------------------------------------------------------------------------///
///                          Connection Information Service                  ///
///--------------------------------------------------------------------------///
class AvailableConnectionsResponse : public UMPS::Python::MessageFormats::IMessage
{
public:
    AvailableConnectionsResponse();
    AvailableConnectionsResponse(const AvailableConnectionsResponse &response);
    explicit AvailableConnectionsResponse(const UMPS::Services::ConnectionInformation::AvailableConnectionsResponse &response);
    AvailableConnectionsResponse(AvailableConnectionsResponse &&response) noexcept;
    AvailableConnectionsResponse& operator=(const AvailableConnectionsResponse &response);
    AvailableConnectionsResponse& operator=(AvailableConnectionsResponse &&response) noexcept;
    [[nodiscard]] const UMPS::Services::ConnectionInformation::AvailableConnectionsResponse &getNativeClassReference() const;
    void setDetails(const std::vector<Details> &details);
    [[nodiscard]] std::vector<Details> getDetails() const noexcept;
    void setReturnCode(UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode code) noexcept;
    [[nodiscard]] UMPS::Services::ConnectionInformation::AvailableConnectionsResponse::ReturnCode getReturnCode() const noexcept;
    ~AvailableConnectionsResponse();
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::AvailableConnectionsResponse> pImpl;
};
/// @class RequestorOptions
class RequestorOptions
{
public:
    RequestorOptions();
    RequestorOptions(const RequestorOptions &options);
    RequestorOptions(RequestorOptions &&options) noexcept;
    RequestorOptions(const UMPS::Services::ConnectionInformation::RequestorOptions &options);
    RequestorOptions& operator=(const RequestorOptions &options);
    RequestorOptions& operator=(RequestorOptions &&options) noexcept;
    RequestorOptions& operator=(const UMPS::Services::ConnectionInformation::RequestorOptions &options);
    [[nodiscard]] const UMPS::Services::ConnectionInformation::RequestorOptions &getNativeClassReference() const;
    void setAddress(const std::string &address);
    [[nodiscard]] std::string getAddress() const;
    void setZAPOptions(const UMPS::Python::Authentication::ZAPOptions &options);
    [[nodiscard]] UMPS::Python::Authentication::ZAPOptions getZAPOptions() const;
    void setTimeOut(const std::chrono::milliseconds &timeOut);
    [[nodiscard]] std::chrono::milliseconds getTimeOut() const;
    void clear() noexcept;
    ~RequestorOptions();
private:
    std::unique_ptr<UMPS::Services::ConnectionInformation::RequestorOptions> pImpl;
};
}
void initialize(pybind11::module &m);
}
#endif
