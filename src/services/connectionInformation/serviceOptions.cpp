#include <iostream>
#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/services/connectionInformation/serviceOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class ServiceOptions::ServiceOptionsImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::string mClientAddress; 
    UMPS::Logging::Level mVerbosity = UMPS::Logging::Level::ERROR;
};

/// C'tor
ServiceOptions::ServiceOptions() :
    pImpl(std::make_unique<ServiceOptionsImpl> ())
{
}

/// Copy assignment
ServiceOptions::ServiceOptions(const ServiceOptions &options)
{
    *this = options;
}

/// Move assignment
ServiceOptions::ServiceOptions(ServiceOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ServiceOptions& ServiceOptions::operator=(const ServiceOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ServiceOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ServiceOptions& ServiceOptions::operator=(ServiceOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Clear
void ServiceOptions::clear() noexcept
{
    pImpl->mClientAddress.clear();
    pImpl->mVerbosity = UMPS::Logging::Level::ERROR;
}

/// Destrutcor
ServiceOptions::~ServiceOptions() = default;

/// Name
std::string ServiceOptions::getName() noexcept
{
   return "ConnectionInformation";
}

/// Client address
void ServiceOptions::setClientAccessAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("Client address is empty");
    }
    pImpl->mClientAddress = address;
}

std::string ServiceOptions::getClientAccessAddress() const
{
    if (!haveClientAccessAddress())
    {
        throw std::runtime_error("Client address not yet set");
    }
    return pImpl->mClientAddress;
}

bool ServiceOptions::haveClientAccessAddress() const noexcept
{
    return !pImpl->mClientAddress.empty();
}

/// Verbosity
void ServiceOptions::setVerbosity(const UMPS::Logging::Level verbosity) noexcept
{
    pImpl->mVerbosity = verbosity;
}

UMPS::Logging::Level ServiceOptions::getVerbosity() const noexcept
{
    return pImpl->mVerbosity;
}

/// ZAP Options
void ServiceOptions::setZAPOptions(const UAuth::ZAPOptions &zapOptions) noexcept
{
    pImpl->mZAPOptions = zapOptions;
}

UAuth::ZAPOptions ServiceOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

void ServiceOptions::parseInitializationFile(const std::string &iniFile,
                                             const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file: "
                                  + iniFile + " does not exist");
    }
    ServiceOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);

    auto clientAccessAddress
        = propertyTree.get<std::string> (section + ".clientAccessAddress",
                                         "");
    if (!clientAccessAddress.empty())
    {
        options.setClientAccessAddress(clientAccessAddress);
    }

    auto defaultVerbosity = static_cast<int> (options.getVerbosity());
    auto verbosity = propertyTree.get<int> (section + ".verbosity",
                                            defaultVerbosity);
    options.setVerbosity(static_cast<UMPS::Logging::Level> (verbosity));
    // Got everything and didn't throw -> copy to this
    *this = std::move(options);
}

