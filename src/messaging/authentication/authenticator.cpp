#include "umps/messaging/authentication/authenticator.hpp"

using namespace UMPS::Messaging::Authentication;

std::string IAuthenticator::okayStatus() noexcept
{
    return "200";
}

std::string IAuthenticator::okayMessage() noexcept
{
    return "OK";
}

std::string IAuthenticator::clientErrorStatus() noexcept
{
    return "400";
}

std::string IAuthenticator::serverErrorStatus() noexcept
{
    return "500";
}


