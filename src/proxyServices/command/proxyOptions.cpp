#include <vector>
#include <chrono>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/proxyServices/command/proxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Command;
namespace UAuth = UMPS::Authentication;

class ProxyOptions::ProxyOptionsImpl
{
public:
    std::vector<std::chrono::milliseconds> mPingIntervals
    {
        std::chrono::milliseconds  {15000}, // 15s
        std::chrono::milliseconds  {30000}, // 30s
        std::chrono::milliseconds  {60000}  // 60s
    };
    UAuth::ZAPOptions mZAPOptions;
    std::string mFrontendAddress;
    std::string mBackendAddress;
    int mBackendHighWaterMark{0};
    int mFrontendHighWaterMark{0};
};

/// C'tor
ProxyOptions::ProxyOptions() :
    pImpl(std::make_unique<ProxyOptionsImpl> ())
{
}

/// Copy c'tor
ProxyOptions::ProxyOptions(const ProxyOptions &options)
{
    *this = options;
}

/// Move c'tor
ProxyOptions::ProxyOptions(ProxyOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
ProxyOptions&
ProxyOptions::operator=(const ProxyOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<ProxyOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
ProxyOptions&
ProxyOptions::operator=(ProxyOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset
void ProxyOptions::clear() noexcept
{
    pImpl = std::make_unique<ProxyOptionsImpl> ();
}

/// Destructor
ProxyOptions::~ProxyOptions() = default;

/// Frontend HWM
void ProxyOptions::setFrontendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mFrontendHighWaterMark = hwm;
}

int ProxyOptions::getFrontendHighWaterMark() const noexcept
{
    return pImpl->mFrontendHighWaterMark;
}

/// Backend HWM
void ProxyOptions::setBackendHighWaterMark(const int hwm)
{
    if (hwm < 0)
    {
        throw std::invalid_argument("Highwater mark cannot be negative");
    }
    pImpl->mBackendHighWaterMark = hwm;
}

int ProxyOptions::getBackendHighWaterMark() const noexcept
{
    return pImpl->mBackendHighWaterMark;
}

/// Frontend address
void ProxyOptions::setFrontendAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    pImpl->mFrontendAddress = address;
}

std::string ProxyOptions::getFrontendAddress() const
{
    if (!haveFrontendAddress())
    {
        throw std::invalid_argument("Frontend address not set");
    }
    return pImpl->mFrontendAddress;
}

bool ProxyOptions::haveFrontendAddress() const noexcept
{
    return !pImpl->mFrontendAddress.empty();
}

/// Backend address
void ProxyOptions::setBackendAddress(const std::string &address)
{
    if (isEmpty(address)){throw std::invalid_argument("Address is empty");}
    if (pImpl->mFrontendAddress == address)
    {
        throw std::invalid_argument(
            "Backend address cannot match frontend address");
    }
    pImpl->mBackendAddress = address;
}

std::string ProxyOptions::getBackendAddress() const
{
    if (!haveBackendAddress())
    {
        throw std::invalid_argument("Backend address not set");
    }
    return pImpl->mBackendAddress;
}

bool ProxyOptions::haveBackendAddress() const noexcept
{
    return !pImpl->mBackendAddress.empty();
}

/// ZAP options
void ProxyOptions::setZAPOptions(const UAuth::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
} 

UAuth::ZAPOptions ProxyOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// Ping interval
void ProxyOptions::setPingIntervals(
    const std::vector<std::chrono::milliseconds> &pingIntervals)
{
    if (pingIntervals.empty())
    {
        throw std::invalid_argument("Ping intervals empty");
    }
    constexpr std::chrono::milliseconds zero{0};
    for (const auto &p : pingIntervals)
    {
        if (p <= zero)
        {
            throw std::invalid_argument("All intervals must be positive");
        }
    }
    auto work = pingIntervals;
    std::sort(work.begin(), work.end());
    work.erase(std::unique(work.begin(), work.end()), work.end());
    pImpl->mPingIntervals = work;
}

std::vector<std::chrono::milliseconds>
    ProxyOptions::getPingIntervals() const noexcept
{
    return pImpl->mPingIntervals;
}

/// Read the proxy optoins from an ini file
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

    // Set frontend/backend address
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
    // High-water marks
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

    // Ping intervals:
    // ModuleRegistry.pingInterval_1 = 
    // ModuleRegistry.pingInterval_2 = 
    // .
    // .
    // .  
    auto defaultNaN = std::numeric_limits<int>::lowest();
    std::vector<std::chrono::milliseconds> pingIntervals;
    for (int i = 1; i < 32768; ++i)
    {
        auto pingIntervalName = section + ".pingInterval_" + std::to_string(i);
        auto pingInterval = propertyTree.get<int> (pingIntervalName,
                                                   defaultNaN);
        if (pingInterval <= 0)
        {
            if (pingInterval > defaultNaN)
            {
                throw std::invalid_argument("Ping interval must be postiive");
            }
            break;
        }
        pingIntervals.push_back(std::chrono::milliseconds {pingInterval}); 
    }
    if (!pingIntervals.empty()){options.setPingIntervals(pingIntervals);}
    // Got everything and didn't throw -> copy to this
    *this = std::move(options);
}
