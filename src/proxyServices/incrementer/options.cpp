#include <iostream>
#include <string>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "umps/proxyServices/incrementer/options.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Incrementer;
namespace UAuth = UMPS::Authentication;

class Options::OptionsImpl
{
public:
    UAuth::ZAPOptions mZAPOptions;
    std::filesystem::path mSqlite3FileName{
        std::string(std::getenv("HOME"))
      + "/.local/share/UMPS/tables/counter.sqlite3"};
    std::string mBackendAddress;
    int64_t mInitialValue{0};
    int mIncrement{1};
    bool mDeleteIfExists{false};
    //UMPS::Logging::Level mVerbosity{UMPS::Logging::Level::ERROR};
};

/// C'tor
Options::Options() :
    pImpl(std::make_unique<OptionsImpl> ())
{
}

/// Copy assignment
Options::Options(const Options &options)
{
    *this = options;
}

/// Move assignment
Options::Options(Options &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
Options& Options::operator=(const Options &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<OptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
Options& Options::operator=(Options &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Clear
void Options::clear() noexcept
{
    pImpl = std::make_unique<OptionsImpl> ();
}

/// Destructor
Options::~Options() = default;

/// Name
void Options::setSqlite3FileName(const std::string &fileName)
{
    if (isEmpty(fileName))
    {
        throw std::invalid_argument("Name is all blank");
    }
    std::filesystem::path filePath{fileName};
    if (!std::filesystem::exists(filePath))
    {
        auto parentPath = filePath.parent_path();
        if (!std::filesystem::exists(parentPath))
        {
            if (!std::filesystem::create_directories(parentPath))
            {
                 throw std::runtime_error("Failed to create path: "
                                        + std::string{parentPath});
            }
        }
    }
    pImpl->mSqlite3FileName = fileName;
}

std::string Options::getSqlite3FileName() const noexcept
{
    return pImpl->mSqlite3FileName;
}

/// Sets the backend address
void Options::setBackendAddress(const std::string &address)
{
    if (address.empty()){throw std::invalid_argument("Address is empty");}
    pImpl->mBackendAddress = address;
}

std::string Options::getBackendAddress() const
{
    if (!haveBackendAddress())
    {
        throw std::runtime_error("Backend address not set");
    }
    return pImpl->mBackendAddress; 
}

bool Options::haveBackendAddress() const noexcept
{
    return !pImpl->mBackendAddress.empty();
}

/// Delete SQLite3 file if it exists
void Options::toggleDeleteSqlite3FileIfExists(
    const bool deleteIfExists) noexcept
{
    pImpl->mDeleteIfExists = deleteIfExists;
}

bool Options::deleteSqlite3FileIfExists() const noexcept
{
    return pImpl->mDeleteIfExists;
}

/// Server address
/*
void Options::setServerAccessAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("Server address is empty");
    }
    pImpl->mServerAddress = address;
}

std::string Options::getServerAccessAddress() const
{
    if (!haveServerAccessAddress())
    {
        throw std::runtime_error("Server address not yet set");
    }
    return pImpl->mServerAddress;
}

bool Options::haveServerAccessAddress() const noexcept
{
    return !pImpl->mServerAddress.empty();
}

/// Client address
void Options::setClientAccessAddress(const std::string &address)
{
    if (isEmpty(address))
    {
        throw std::invalid_argument("Client address is empty");
    }
    pImpl->mClientAddress = address;
}

std::string Options::getClientAccessAddress() const
{
    if (!haveClientAccessAddress())
    {
        throw std::runtime_error("Client address not yet set");
    }
    return pImpl->mClientAddress;
}

bool Options::haveClientAccessAddress() const noexcept
{
    return !pImpl->mClientAddress.empty();
}
*/

/// Incrementer increment
void Options::setIncrement(const int increment)
{
    if (increment < 1)
    {
        throw std::invalid_argument("Increment must be positive"); 
    }
    pImpl->mIncrement = increment;
}

int Options::getIncrement() const noexcept
{
    return pImpl->mIncrement;
}

/// Initial value
void Options::setInitialValue(const int32_t value) noexcept
{
    pImpl->mInitialValue = static_cast<int64_t> (value);
}

int64_t Options::getInitialValue() const noexcept
{
    return pImpl->mInitialValue;
}

/*
/// Verbosity
void Options::setVerbosity(const UMPS::Logging::Level verbosity) noexcept
{
    pImpl->mVerbosity = verbosity;
}

UMPS::Logging::Level Options::getVerbosity() const noexcept
{
    return pImpl->mVerbosity;
}
*/

/// ZAP Options
void Options::setZAPOptions(const UAuth::ZAPOptions &zapOptions) noexcept
{
    pImpl->mZAPOptions = zapOptions;
}

UAuth::ZAPOptions Options::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

void Options::parseInitializationFile(const std::string &iniFile,
                                      const std::string &section)
{
    if (!std::filesystem::exists(iniFile))
    {
        throw std::invalid_argument("Initialization file: "
                                  + iniFile + " does not exist");
    }
    Options options;
    boost::property_tree::ptree propertyTree;
    boost::property_tree::ini_parser::read_ini(iniFile, propertyTree);

    auto sqlite3FileName
        = propertyTree.get<std::string> (section + ".sqlite3FileName",
                                         options.getSqlite3FileName());
    options.setSqlite3FileName(sqlite3FileName);
    //auto serverAddress
    //    = propertyTree.get<std::string> (section + ".serverAccessAddress");
    //options.setServerAccessAddress(serverAddress);
    //auto clientAccessAddress
    //    = propertyTree.get<std::string> (section + ".clientAccessAddress",
    //                                     "");
    //if (!clientAccessAddress.empty())
    //{
    //    options.setClientAccessAddress(clientAccessAddress);
    //}

    auto increment = propertyTree.get<int> (section + ".increment", 1);
    options.setIncrement(increment);
    auto initialValue = propertyTree.get<int> (section + ".initialValue", 0);
    options.setInitialValue(initialValue);

    //auto defaultVerbosity = static_cast<int> (options.getVerbosity());
    //auto verbosity = propertyTree.get<int> (section + ".verbosity",
    //                                        defaultVerbosity);
    //options.setVerbosity(static_cast<UMPS::Logging::Level> (verbosity));
    // Got everything and didn't throw -> copy to this
    *this = std::move(options);
}

