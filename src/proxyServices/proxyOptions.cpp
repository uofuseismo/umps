#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string.hpp>
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices;
namespace RouterDealer = UMPS::Messaging::RouterDealer;
namespace UAuth = UMPS::Authentication;

class ProxyOptions::ProxyOptionsImpl
{
public:
    ProxyOptionsImpl()
    {
        mProxyOptions.setFrontendHighWaterMark(8192); //2000);
        mProxyOptions.setBackendHighWaterMark(0); //8192); //1000);
    }
    RouterDealer::ProxyOptions mProxyOptions;
    std::string mName;
};

/// Constructor
ProxyOptions::ProxyOptions() :
    pImpl(std::make_unique<ProxyOptionsImpl> ())
{
}

/// Copy c'tor
[[maybe_unused]]
ProxyOptions::ProxyOptions(const ProxyOptions &options)
{
    *this = options;
}

/// Move c'tor
[[maybe_unused]]
ProxyOptions::ProxyOptions(ProxyOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ProxyOptions& ProxyOptions::operator=(const ProxyOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ProxyOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ProxyOptions& ProxyOptions::operator=(ProxyOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Destructor
ProxyOptions::~ProxyOptions() = default;

/// Reset class
void ProxyOptions::clear() noexcept
{
    pImpl = std::make_unique<ProxyOptionsImpl> ();
}

/// The name of the proxy
void ProxyOptions::setName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("Name is empty");}
    pImpl->mName = name;
}

std::string ProxyOptions::getName() const
{
    if (!haveName()){throw std::runtime_error("Proxy name not set");}
    return pImpl->mName;
}

bool ProxyOptions::haveName() const noexcept
{
    return !pImpl->mName.empty();
}

/// Frontend highwater mark
void ProxyOptions::setFrontendHighWaterMark(const int highWaterMark)
{
    pImpl->mProxyOptions.setFrontendHighWaterMark(highWaterMark);
}

/// Backend highwater mark
void ProxyOptions::setBackendHighWaterMark(const int highWaterMark)
{
    pImpl->mProxyOptions.setBackendHighWaterMark(highWaterMark);
}

/// Sets the frontend address
void ProxyOptions::setFrontendAddress(const std::string &address)
{
    pImpl->mProxyOptions.setFrontendAddress(address);
}

/// Sets the backend adddress
void ProxyOptions::setBackendAddress(const std::string &address)
{
    pImpl->mProxyOptions.setBackendAddress(address);
}

/// ZAP Options
void ProxyOptions::setZAPOptions(const UAuth::ZAPOptions &zapOptions) noexcept
{
    pImpl->mProxyOptions.setZAPOptions(zapOptions);
}

RouterDealer::ProxyOptions ProxyOptions::getProxyOptions() const noexcept
{
    return pImpl->mProxyOptions;
}

/// Read proxy options from an ini file
void ProxyOptions::parseInitializationFile(const std::string &iniFile,
                                           const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file: "
                                  + iniFile + " does not exist");
    }
    ProxyOptions options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);
    auto defaultFrontendHWM
        = options.getProxyOptions().getFrontendHighWaterMark();
    auto defaultBackendHWM
        = options.getProxyOptions().getBackendHighWaterMark();
    // Default name
    std::vector<std::string> splitSection;
    boost::split(splitSection, section, boost::is_any_of(":"));
    std::string defaultName{""};
    if (splitSection.size() >= 2)
    {
        defaultName = splitSection.back();
    }
    /// Get name
    auto name = propertyTree.get<std::string> (section + ".name", defaultName);
    if (!name.empty()){options.setName(name);}

    auto frontendAddress
        = propertyTree.get<std::string> (section + ".frontendAddress",
                                         "");
    if (!frontendAddress.empty())
    {
        options.setFrontendAddress(frontendAddress);
    }

    auto backendAddress
        = propertyTree.get<std::string> (section + ".backendAddress",
                                         "");
    if (!backendAddress.empty())
    {
        options.setBackendAddress(backendAddress);
    }

    auto frontendHighWaterMark
       = propertyTree.get<int> (section + ".frontendHighWaterMark",
                                defaultFrontendHWM);
    if (frontendHighWaterMark >= 0)
    {
        options.setFrontendHighWaterMark(frontendHighWaterMark);
    }

    auto backendHighWaterMark
       = propertyTree.get<int> (section + ".backendHighWaterMark",
                                defaultBackendHWM);
    if (backendHighWaterMark >= 0)
    {
        options.setBackendHighWaterMark(backendHighWaterMark);
    }

    // Got everything and didn't throw -> copy to this
    *this = std::move(options);

}
