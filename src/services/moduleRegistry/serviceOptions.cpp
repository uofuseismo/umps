#include <iostream>
#include <string>
#include <filesystem>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/ini_parser.hpp>
#include "umps/services/moduleRegistry/serviceOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ModuleRegistry;
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
ServiceOptions::ServiceOptions(const ServiceOptions &parameters)
{
    *this = parameters;
}

/// Move assignment
ServiceOptions::ServiceOptions(ServiceOptions &&parameters) noexcept
{
    *this = std::move(parameters);
}

/// Copy assignment
ServiceOptions& ServiceOptions::operator=(const ServiceOptions &parameters)
{
    if (&parameters == this){return *this;}
    pImpl = std::make_unique<ServiceOptionsImpl> (*parameters.pImpl);
    return *this;
}

/// Move assignment
ServiceOptions& ServiceOptions::operator=(ServiceOptions &&parameters) noexcept
{
    if (&parameters == this){return *this;}
    pImpl = std::move(parameters.pImpl);
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
   return "ModuleRegistry";
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

/*
void ServiceOptions::parseInitializationFile(const std::string &iniFile,
                                         const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file: "
                                  + iniFile + " does not exist");
    }
    ServiceOptions parameters;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);

    auto clientAccessAddress
        = propertyTree.get<std::string> (section + ".clientAccessAddress",
                                         "");
    if (!clientAccessAddress.empty())
    {
        parameters.setClientAccessAddress(clientAccessAddress);
    }

    auto defaultVerbosity = static_cast<int> (parameters.getVerbosity());
    auto verbosity = propertyTree.get<int> (section + ".verbosity",
                                            defaultVerbosity);
    parameters.setVerbosity(static_cast<UMPS::Logging::Level> (verbosity));
    // Got everything and didn't throw -> copy to this
    *this = std::move(parameters);
}
*/
