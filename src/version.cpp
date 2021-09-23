#include <string>
#include "umps/version.hpp"

using namespace UMPS;

int Version::getMajor() noexcept
{
    return UMPS_MAJOR;
}

int Version::getMinor() noexcept
{
    return UMPS_MINOR;
}

int Version::getPatch() noexcept
{
    return UMPS_PATCH;
}

bool Version::isAtLeast(const int major, const int minor,
                        const int patch) noexcept
{
    if (UMPS_MAJOR < major){return false;}
    if (UMPS_MAJOR > major){return true;}
    if (UMPS_MINOR < minor){return false;}
    if (UMPS_MINOR > minor){return true;}
    if (UMPS_PATCH < patch){return false;}
    return true;
}

std::string Version::getVersion() noexcept
{
    std::string version(UMPS_VERSION);
    return version;
}
