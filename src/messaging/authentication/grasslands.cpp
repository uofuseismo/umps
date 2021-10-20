#include "umps/messaging/authentication/grasslands.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"

using namespace UMPS::Messaging::Authentication;

Grasslands::~Grasslands() = default;
    
std::pair<std::string, std::string> Grasslands::isBlacklisted(
        const std::string &address) const noexcept
{
    return std::pair(okayStatus(), okayMessage());
}   

std::pair<std::string, std::string> Grasslands::isWhitelisted(
        const std::string &) const noexcept
{
    return std::pair(okayStatus(), okayMessage());
}

std::pair<std::string, std::string> Grasslands::isValid(
    const Certificate::UserNameAndPassword &) const noexcept
{
    return std::pair(okayStatus(), okayMessage());
}

std::pair<std::string, std::string> Grasslands::isValid(
    const Certificate::Keys &) const noexcept
{
    return std::pair(okayStatus(), okayMessage());
}   
