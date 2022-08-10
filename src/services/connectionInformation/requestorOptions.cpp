#include <string>
#include <chrono>
#include <filesystem>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/services/connectionInformation/requestorOptions.hpp"
#include "umps/services/connectionInformation/availableConnectionsResponse.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/requestRouter/requestOptions.hpp"
#include "umps/modules/operator/readZAPOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;
namespace URequestRouter = UMPS::Messaging::RequestRouter;

class RequestorOptions::RequestorOptionsImpl
{
public:
    RequestorOptionsImpl()
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
RequestorOptions::RequestorOptions() :
    pImpl(std::make_unique<RequestorOptionsImpl> ())
{
}

/// Copy c'tor
RequestorOptions::RequestorOptions(const RequestorOptions &options)
{
    *this = options;
}

/// Move c'tor
RequestorOptions::RequestorOptions(RequestorOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RequestorOptions& RequestorOptions::operator=(const RequestorOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RequestorOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RequestorOptions& RequestorOptions::operator=(RequestorOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
RequestorOptions::~RequestorOptions() = default;

/// Reset class
void RequestorOptions::clear() noexcept
{
    pImpl = std::make_unique<RequestorOptionsImpl> ();
}

/// End point
void RequestorOptions::setAddress(const std::string &address)
{
    pImpl->mRequestOptions.setAddress(address);
}

/// ZAP options
void RequestorOptions::setZAPOptions(const UAuth::ZAPOptions &zapOptions)
{
    pImpl->mRequestOptions.setZAPOptions(zapOptions);
}

/// Time out
void RequestorOptions::setTimeOut(
    const std::chrono::milliseconds timeOut) noexcept
{
    pImpl->mRequestOptions.setTimeOut(timeOut);
}

/// Native class options
URequestRouter::RequestOptions
RequestorOptions::getRequestOptions() const noexcept
{
    return pImpl->mRequestOptions;
}

/// Load from ini file
void RequestorOptions::parseInitializationFile(const std::string &iniFile,
                                               const std::string &section)
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
         (section + ".address", "");
    if (isEmpty(operatorAddress))
    {
        operatorAddress = propertyTree.get<std::string>
            (section + ".ipAddress", "");
        if (isEmpty(operatorAddress))
        {
            throw std::runtime_error("Operator address not set");
        }
    }
    RequestorOptions options;
    options.setAddress(operatorAddress);
    // Load the ZAP options
    auto zapOptions
         = UMPS::Modules::Operator::readZAPClientOptions(iniFile, section);
    options.setZAPOptions(zapOptions); 
    // Save it
    *this = options;
}
