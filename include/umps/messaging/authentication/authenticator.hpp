#ifndef UMPS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#define UMPS_MESSAGING_AUTHENTICATION_AUTHENTICATOR_HPP
#include <memory>
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Messaging::Authentication::Certificate
{
 class Keys;
 class UserNameAndPassword;
}
namespace UMPS::Messaging::Authentication
{
/// @class Authenticator "authenticator.hpp" "umps/messaging/authentication/authenticator.hpp"
/// @brief This the abstract base class that defines an authenticator.  
/// @note This is based on the pyzmq implementation in base.py
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IAuthenticator
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    //IAuthenticator();
    /// @brief Constructor with a specified logger.
    /// @param[in] logger   The logging utility.
    //explicit Authenticator(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a specified context.
    /// @param[in] context  The ZeroMQ context to on which to open a socket.
    //explicit Authenticator(std::shared_ptr<zmq::context_t> &context);
    /// @brief Initializes with a specified context and logger.
    /// @param[in] context  The ZeroMQ context to on which to open a socket.
    /// @param[in] logger   The logging utility.
    //Authenticator(std::shared_ptr<zmq::context_t> &context,
    //              std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

/*
    /// @name White and Blacklisting
    /// @{
    /// @brief Denies access to a certain IP address.
    /// @param[in] address  The address to add to the blacklist.
    /// @throws std::invalid_argument if the address is whitelisted or empty.
    void addToBlacklist(const std::string &address);
    /// @brief Removes an IP address from the blacklist.
    /// @param[in] address  The address to remove from the blacklist.
    void removeFromBlacklist(const std::string &address) noexcept;
*/
    /// @result True indicates the address is blacklisted.
    [[nodiscard]] virtual bool isBlacklisted(const std::string &address) const noexcept = 0;

/*
    /// @brief Grants access to a certain IP address.
    /// @param[in] address  The address to add to the whitelist.
    /// @throws std::invalid_argument if the address is blacklisted or empty.
    void addToWhitelist(const std::string &address);
    /// @brief Removes an IP address from the whitelist.
    /// @param[in] address  The address to remove from the whitelist.
    void removeFromWhitelist(const std::string &address) noexcept;
*/
    /// @result True indicates the address is whitelisted.
    [[nodiscard]] virtual bool isWhitelisted(const std::string &address) const noexcept = 0;
    /// @}


    /// @brief Creates and binds the ZAP socket.
    //void start();

    /// @brief Closes the ZAP socket.
    //void stop();

    //zmq::socket_t* getZapSocket();

    /// @name Destructor
    /// @{
    /// @brief Destructor.
    //virtual ~IAuthenticator() = default;
    /// @}

//    Authenticator(const Authenticator &authenticator) = delete;
    //Authenticator& operator=(const Authenticator &authenticator) = delete;
/*
private:
    class AuthenticatorImpl;
    std::unique_ptr<AuthenticatorImpl> pImpl;
*/
};
}
#endif
