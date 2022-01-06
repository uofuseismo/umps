#include <iostream>
#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/services/incrementer/parameters.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Incrementer;
namespace UAuth = UMPS::Authentication;

class Parameters::ParametersImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::string mName;
    //std::string mServerAddress;
    std::string mClientAddress; 
    uint64_t mInitialValue = 0;
    uint64_t mIncrement = 1;
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
    pImpl->mName.clear();
    //pImpl->mServerAddress.clear();
    pImpl->mClientAddress.clear();
    pImpl->mInitialValue = 0;
    pImpl->mIncrement = 1;
    pImpl->mVerbosity = UMPS::Logging::Level::ERROR;
}

/// Destrutcor
Parameters::~Parameters() = default;

/// Name
void Parameters::setName(const std::string &name)
{
   if (isEmpty(name))
   {
       throw std::invalid_argument("Name is all blank");
   }
   pImpl->mName = name;
}

std::string Parameters::getName() const
{
   if (!haveName()){throw std::runtime_error("Name not yet set");}
   return pImpl->mName;
}

bool Parameters::haveName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Server address
/*
void Parameters::setServerAccessAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("Server address is empty");
    }
    pImpl->mServerAddress = address;
}

std::string Parameters::getServerAccessAddress() const
{
    if (!haveServerAccessAddress())
    {
        throw std::runtime_error("Server address not yet set");
    }
    return pImpl->mServerAddress;
}

bool Parameters::haveServerAccessAddress() const noexcept
{
    return !pImpl->mServerAddress.empty();
}
*/

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

/// Incrementer increment
void Parameters::setIncrement(const int increment)
{
    if (increment < 1)
    {
        throw std::invalid_argument("Increment must be positive"); 
    }
    pImpl->mIncrement = static_cast<uint64_t> (increment);
}

uint64_t Parameters::getIncrement() const noexcept
{
    return pImpl->mIncrement;
}

/// Initial value
void Parameters::setInitialValue(const uint32_t value) noexcept
{
    pImpl->mInitialValue = static_cast<uint64_t> (value);
}

uint64_t Parameters::getInitialValue() const noexcept
{
    return pImpl->mInitialValue;
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

    auto name = propertyTree.get<std::string> (section + ".name");
    parameters.setName(name);
    //auto serverAddress
    //    = propertyTree.get<std::string> (section + ".serverAccessAddress");
    //parameters.setServerAccessAddress(serverAddress);
    auto clientAccessAddress
        = propertyTree.get<std::string> (section + ".clientAccessAddress",
                                         "");
    if (!clientAccessAddress.empty())
    {
        parameters.setClientAccessAddress(clientAccessAddress);
    }

    auto increment = propertyTree.get<int> (section + ".increment", 1);
    parameters.setIncrement(increment);
    auto initialValue = propertyTree.get<int> (section + ".initialValue", 0);
    parameters.setInitialValue(initialValue);

    auto defaultVerbosity = static_cast<int> (parameters.getVerbosity());
    auto verbosity = propertyTree.get<int> (section + ".verbosity",
                                            defaultVerbosity);
    parameters.setVerbosity(static_cast<UMPS::Logging::Level> (verbosity));
    // Got everything and didn't throw -> copy to this
    *this = std::move(parameters);
}

