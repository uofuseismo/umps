#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/proxyServices/packetCache/proxyOptions.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::PacketCache;
namespace UAuth = UMPS::Authentication;

class ProxyOptions::ProxyOptionsImpl
{
public:
    ProxyOptionsImpl()
    {
        mProxyOptions.setFrontendHighWaterMark(1000);
        mProxyOptions.setBackendHighWaterMark(1000);
        //mProxyOptions.setTopic("PacketCache");
    }
    UMPS::Messaging::RouterDealer::ProxyOptions mProxyOptions;
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

/// Proxy service name
void ProxyOptions::setName(const std::string &name)
{
    if (isEmpty(name)){throw std::invalid_argument("Name is empty");}
    pImpl->mName = name;
}

std::string ProxyOptions::getName() const
{
    if (!haveName()){throw std::runtime_error("Name not yet set");}
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

int ProxyOptions::getFrontendHighWaterMark() const noexcept
{
    return pImpl->mProxyOptions.getFrontendHighWaterMark();
}

/// Backend highwater mark
void ProxyOptions::setBackendHighWaterMark(const int highWaterMark)
{
    pImpl->mProxyOptions.setBackendHighWaterMark(highWaterMark);
}

int ProxyOptions::getBackendHighWaterMark() const noexcept
{
    return pImpl->mProxyOptions.getBackendHighWaterMark();
}

/// Sets the frontend address
void ProxyOptions::setFrontendAddress(const std::string &address)
{
    pImpl->mProxyOptions.setFrontendAddress(address);
}

std::string ProxyOptions::getFrontendAddress() const
{
    return pImpl->mProxyOptions.getFrontendAddress();
}

bool ProxyOptions::haveFrontendAddress() const noexcept
{
    return pImpl->mProxyOptions.haveFrontendAddress();
}

/// Sets the backend adddress
void ProxyOptions::setBackendAddress(const std::string &address)
{
    pImpl->mProxyOptions.setBackendAddress(address);
}

std::string ProxyOptions::getBackendAddress() const
{
    return pImpl->mProxyOptions.getBackendAddress();
}

bool ProxyOptions::haveBackendAddress() const noexcept
{
    return pImpl->mProxyOptions.haveBackendAddress();
}

/// ZAP Options
void ProxyOptions::setZAPOptions(const UAuth::ZAPOptions &zapOptions) noexcept
{
    pImpl->mProxyOptions.setZAPOptions(zapOptions);
}

UAuth::ZAPOptions ProxyOptions::getZAPOptions() const noexcept
{
    return pImpl->mProxyOptions.getZAPOptions();
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
                                options.getFrontendHighWaterMark());
    if (frontendHighWaterMark >= 0)
    {
        options.setFrontendHighWaterMark(frontendHighWaterMark);
    }

    auto backendHighWaterMark
       = propertyTree.get<int> (section + ".backendHighWaterMark",
                                options.getBackendHighWaterMark());
    if (backendHighWaterMark >= 0)
    {
        options.setBackendHighWaterMark(backendHighWaterMark);
    }

    // Got everything and didn't throw -> copy to this
    *this = std::move(options);

}
