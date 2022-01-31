#include <string>
#include <chrono>
#include <filesystem>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/services/connectionInformation/requestOptions.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class RequestOptions::RequestOptionsImpl
{
public:
    RequestOptionsImpl()
    {
        // Tell request types of messages we can unpack 
        std::unique_ptr<UMPS::MessageFormats::IMessage> responseMessage
            = std::make_unique<AvailableConnectionsResponse> (); 
        mRequestOptions.addMessageFormat(responseMessage);
        mRequestOptions.setTimeOut(std::chrono::seconds{5});
    }
    URequestRouter::RequestOptions mRequestOptions;    
};

/// C'tor
RequestOptions::RequestOptions() :
    pImpl(std::make_unique<RequestOptionsImpl> ())
{
}

/// Copy c'tor
RequestOptions::RequestOptions(const RequestOptions &options)
{
    *this = options;
}

/// Move c'tor
RequestOptions::RequestOptions(RequestOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RequestOptions& RequestOptions::operator=(const RequestOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RequestOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RequestOptions& RequestOptions::operator=(RequestOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
RequestOptions::~RequestOptions() = default;

/// Reset class
void RequestOptions::clear() noexcept
{
    pImpl = std::make_unique<RequestOptionsImpl> ();
}

/// End point
void RequestOptions::setEndPoint(const std::string &endPoint)
{
    pImpl->mRequestOptions.setEndPoint(endPoint);
}

/// ZAP options
void RequestOptions::setZAPOptions(const UAuth::ZAPOptions &zapOptions)
{
    pImpl->mRequestOptions.setZAPOptions(zapOptions);
}

/// Time out
void RequestOptions::setTimeOut(
    const std::chrono::milliseconds timeOut) noexcept
{
    pImpl->mRequestOptions.setTimeOut(timeOut);
}

/// Native class options
URequestRouter::RequestOptions
RequestOptions::getRequestOptions() const noexcept
{
    return pImpl->mRequestOptions;
}

/// Load from ini file
void RequestOptions::parseInitializationFile(const std::string &iniFile)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file: "
                                  + iniFile + " does not exist");
    }
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    // Load the operator's front-end address
    auto operatorAddress = propertyTree.get<std::string>
         ("uOperator.ipAddress", "");
    if (isEmpty(operatorAddress))
    {
        throw std::runtime_error("Operator address not set");
    }
    RequestOptions options;
    options.setEndPoint(operatorAddress);
    // Load the ZAP options
    auto zapOptions
         = UMPS::Modules::Operator::readZAPClientOptions(propertyTree);
    options.setZAPOptions(zapOptions); 
    // Save it
    *this = options;
}
