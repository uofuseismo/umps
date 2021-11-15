#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/broadcasts/dataPacket/parameters.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"

using namespace UMPS::Broadcasts::DataPacket;

class Parameters::ParametersImpl
{
public:
    ParametersImpl()
    {
        mProxyOptions.setFrontendHighWaterMark(2000);
        mProxyOptions.setBackendHighWaterMark(1000);
        mProxyOptions.setTopic("DataPacket");
    }
    UMPS::Messaging::XPublisherXSubscriber::ProxyOptions mProxyOptions;
};

/// Constructor
Parameters::Parameters() :
    pImpl(std::make_unique<ParametersImpl> ())
{
}

/// Copy c'tor
Parameters::Parameters(const Parameters &parameters)
{
    *this = parameters;
}

/// Move c'tor
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

/// Destructor
Parameters::~Parameters() = default;

/// Reset class
void Parameters::clear() noexcept
{
    pImpl = std::make_unique<ParametersImpl> ();
}

/// Broadcast name
std::string Parameters::getName() noexcept
{
    return "DataPacket";
}

/// Frontend highwater mark
void Parameters::setFrontendHighWaterMark(const int highWaterMark)
{
    pImpl->mProxyOptions.setFrontendHighWaterMark(highWaterMark);
}

int Parameters::getFrontendHighWaterMark() const noexcept
{
    return pImpl->mProxyOptions.getFrontendHighWaterMark();
}

/// Backend highwater mark
void Parameters::setBackendHighWaterMark(const int highWaterMark)
{
    pImpl->mProxyOptions.setBackendHighWaterMark(highWaterMark);
}

int Parameters::getBackendHighWaterMark() const noexcept
{
    return pImpl->mProxyOptions.getBackendHighWaterMark();
}

/// Sets the frontend address
void Parameters::setFrontendAddress(const std::string &address)
{
    pImpl->mProxyOptions.setFrontendAddress(address);
}

std::string Parameters::getFrontendAddress() const
{
    return pImpl->mProxyOptions.getFrontendAddress();
}

bool Parameters::haveFrontendAddress() const noexcept
{
    return pImpl->mProxyOptions.haveFrontendAddress();
}

/// Sets the backend adddress
void Parameters::setBackendAddress(const std::string &address)
{
    pImpl->mProxyOptions.setBackendAddress(address);
}

std::string Parameters::getBackendAddress() const
{
    return pImpl->mProxyOptions.getBackendAddress();
}

bool Parameters::haveBackendAddress() const noexcept
{
    return pImpl->mProxyOptions.haveBackendAddress();
}

/// Read parameters from an ini file
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

    auto frontendAddress
        = propertyTree.get<std::string> (section + ".frontendAddress",
                                         "");
    if (!frontendAddress.empty())
    {
        parameters.setFrontendAddress(frontendAddress);
    }

    auto backendAddress
        = propertyTree.get<std::string> (section + ".backendAddress",
                                         "");
    if (!backendAddress.empty())
    {
        parameters.setBackendAddress(backendAddress);
    }

    auto frontendHighWaterMark
       = propertyTree.get<int> (section + ".frontendHighWaterMark",
                                parameters.getFrontendHighWaterMark());
    if (frontendHighWaterMark >= 0)
    {
        parameters.setFrontendHighWaterMark(frontendHighWaterMark);
    }

    auto backendHighWaterMark
       = propertyTree.get<int> (section + ".backendHighWaterMark",
                                parameters.getBackendHighWaterMark());
    if (backendHighWaterMark >= 0)
    {
        parameters.setBackendHighWaterMark(backendHighWaterMark);
    }


    // Got everything and didn't throw -> copy to this
    *this = std::move(parameters);

}
