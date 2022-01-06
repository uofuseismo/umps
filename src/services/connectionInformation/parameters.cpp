#include <iostream>
#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/services/connectionInformation/parameters.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::ConnectionInformation;
namespace UAuth = UMPS::Authentication;

class Parameters::ParametersImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::string mClientAddress; 
    UMPS::Logging::Level mVerbosity = UMPS::Logging::Level::ERROR;
};

/// C'tor
Parameters::Parameters() :
    pImpl(std::make_unique<ParametersImpl> ())
{
}

/// Copy assignment
Parameters::Parameters(const Parameters &parameters)
{
    *this = parameters;
}

/// Move assignment
Parameters::Parameters(Parameters &&parameters) noexcept
{
    *this = std::move(parameters);
}

/// Copy assignment
Parameters& Parameters::operator=(const Parameters &parameters)
{
    if (&parameters == this){return *this;}
    pImpl = std::make_unique<ParametersImpl> (*parameters.pImpl);
    return *this;
}

/// Move assignment
Parameters& Parameters::operator=(Parameters &&parameters) noexcept
{
    if (&parameters == this){return *this;}
    pImpl = std::move(parameters.pImpl);
    return *this;
}

/// Clear
void Parameters::clear() noexcept
{
    pImpl->mClientAddress.clear();
    pImpl->mVerbosity = UMPS::Logging::Level::ERROR;
}

/// Destrutcor
Parameters::~Parameters() = default;

/// Name
std::string Parameters::getName() noexcept
{
   return "ConnectionInformation";
}

/// Client address
void Parameters::setClientAccessAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("Client address is empty");
    }
    pImpl->mClientAddress = address;
}

std::string Parameters::getClientAccessAddress() const
{
    if (!haveClientAccessAddress())
    {
        throw std::runtime_error("Client address not yet set");
    }
    return pImpl->mClientAddress;
}

bool Parameters::haveClientAccessAddress() const noexcept
{
    return !pImpl->mClientAddress.empty();
}

/// Verbosity
void Parameters::setVerbosity(const UMPS::Logging::Level verbosity) noexcept
{
    pImpl->mVerbosity = verbosity;
}

UMPS::Logging::Level Parameters::getVerbosity() const noexcept
{
    return pImpl->mVerbosity;
}

/// ZAP Options
void Parameters::setZAPOptions(const UAuth::ZAPOptions &zapOptions) noexcept
{
    pImpl->mZAPOptions = zapOptions;
}

UAuth::ZAPOptions Parameters::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

void Parameters::parseInitializationFile(const std::string &iniFile,
                                         const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file: "
                                  + iniFile + " does not exist");
    }
    Parameters parameters;
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

