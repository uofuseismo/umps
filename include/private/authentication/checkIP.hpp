#ifndef PRIVATE_AUTHENTICATION_CHECKIP_HPP
#define PRIVATE_AUTHENTICATION_CHECKIP_HPP
#include <set>
#include <string>
#include <cstring>
#include "private/isEmpty.hpp"
namespace
{
/// @brief Checks if an IP address matches the list of IP addresses.
///        This is slightly more complicated since we allow wildcards.
[[maybe_unused]]
[[nodiscard]]
bool ipExists(const std::string &ip,
              const std::set<std::string> &ipAddresses)
{
    // Most straightforward thing is IP is actually in list
    if (ipAddresses.contains(ip)){return true;}
    // Everything - no wild-carding
    if (ipAddresses.contains("*.*.*.*")){return true;}
    // Now we need to try a bit harder
    for (const auto &address : ipAddresses)
    {
        // Is this wildcarded?
        // This should match things like 127.23.34.55 == 127.*
        auto found = address.find_first_of('*');
        if (found != std::string::npos && found > 0)
        {
            if (std::strncmp(address.c_str(), ip.c_str(), found - 1) == 0)
            {
               return true;
            }
        }
    }
    return false;
}
/// @brief Determines if I can parse the given IP address
bool isOkayIP(const std::string &ip)
{
    // String is empty - no
    if (isEmpty(ip)){return false;}
    if (ip.size() == 0){return false;} // Double check on empty
    if (ip == "*.*.*.*"){return true;} // Total wild card is okay
    if (ip[0] == '*'){return false;} // First character wildcard is bad
    // Can only handle one wildcard
    if (std::count(ip.begin(), ip.end(), '*') > 1){return false;}
    return true;
}
}
#endif
