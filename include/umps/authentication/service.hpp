#ifndef UMPS_AUTHENTICATION_THREAD_AUTHENTICATOR_HPP
#define UMPS_AUTHENTICATION_THREAD_AUTHENTICATOR_HPP
#include <memory>
#include <vector>
#include <string>
#include "umps/authentication/authenticator.hpp"
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Messaging
 {
  class Context;
 }
}
namespace UMPS::Authentication
{
/// @class Service "service.hpp" "umps/authentication/service.hpp"
/// @brief This class is used to run a ZAP authentication thread in
///        the background.
/// @details Effectively when you \c start() the in a thread is running an
///          UMPS::Messaging::Authentication::Authenticator.  You modify the
///          state of this Authenticator by issuing a service request.  This
///          request is transmitted to the thread that strated the service.
/// @note This draws ideas from auth/thread.py in the Python ZMQ implementation
///       as well as brettviren's cppzmq-houses examples.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Authentication_Authenticator
class Service
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Service();
    /// @brief Constructor with a specified logger.
    /// @param[in] logger   The logging utility.
    explicit Service(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a specified context.
    /// @param[in] context  The ZeroMQ context to on which to open a socket.
    explicit Service(std::shared_ptr<UMPS::Messaging::Context> &context);
    /// @brief Constructor with a specified authenticator
    explicit Service(std::shared_ptr<IAuthenticator> &authenticator);
    /// @brief Initializes with a specified context and logger.
    /// @param[in] context  The ZeroMQ context to on which to open a socket.
    /// @param[in] logger   The logging utility.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Initializes with a specified logger and authenticator.
    /// @param[in] logger         The logging utility.
    /// @param[in] authenticator  The authentication utility.
    Service(std::shared_ptr<UMPS::Logging::ILog> &logger,
            std::shared_ptr<IAuthenticator> &authenticator);
    /// @brief Initializes with a specified context and authenticator.
    /// @param[in] context        The ZeroMQ context to on which to open a
    ///                           socket.
    /// @param[in] authenticator  The authentication utility.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<IAuthenticator> &authenticator);
    /// @brief Initializes with a specified context, logger, and authenticator.
    /// @param[in] context        The ZeroMQ context to on which to open a
    ///                           socket.
    /// @param[in] logger         The logging utility.
    /// @param[in] authenticator  The authentication utility.
    Service(std::shared_ptr<UMPS::Messaging::Context> &context,
            std::shared_ptr<UMPS::Logging::ILog> &logger,
            std::shared_ptr<IAuthenticator> &authenticator);
    /// @}


    /// @name Service State
    /// @{

    /// @brief Starts the authentication thread.
    /// @note You would start this service by doing the following:
    ///         // Start the service 
    ///         namespace UMPSAuth = UMPS::Messaging::Authentication; 
    ///         UMPSAuth::ThreadAuthentictor auth;
    ///         std::thread t(&UMPSAuth::Service::start,
    ///                       &auth);
    ///         // Main thread configures authorization service
    ///         auth.whitelist(addresses); // Allow some addresses
    ///         auth.blacklist(addresses); // Deny some addresses
    ///         .
    ///         .
    ///         .
    ///         auth.stop(); // Main thread stops service
    ///         t.join(); // Thread t returns
    void start();
    /// @result True indicates that the authentication service is running.
    [[nodiscard]] bool isRunning() const noexcept;
    /// @brief Stops the authentciation thread.
    void stop();
    /// @}

    /// @name Service Requests
    /// @{

    /// @brief Whitelists (allows) connections from the given IP address.
    /// @param[in] address  The address from which to allow connections.
    /// @throws std::runtime_error if \c isRunning() is false.
    /// @throws std::invalid_argument if address is empty.
    void whitelist(const std::string &address);
    /// @brief Whitelists (allows) connections from IP addresses.
    /// @param[in] addresses  The addresses from which to allow connections.
//    void whitelist(const std::vector<std::string> &addresses);

    /// @brief Blacklists (denies) connections from the given IP address.
    /// @param[in] address  The address from which to deny connections.
    /// @throws std::runtime_error if \c isRunning is false.
    /// @throws std::invalid_argument if address is emtpy.
    void blacklist(const std::string &address);
    /// @brief Blacklists (denies) connections from IP address(es).
    /// @param[in] address  The IP address to deny access to.
    /// @note This is mutually exclusive from the whitelist.
//    void blacklist(const std::vector<std::string> &addresses);
    /// @brief Configure CURVE PLAIN authentication for a given domain.
    /// @param[in] domain     The directory (table) containing the password
    ///                       file.  To cover all domains use "*".
    /// @param[in] passwords  The passwords for this domain.
    void configurePlainTextPasswords(const std::string &domain, 
                                     const std::vector<std::string> &passwords);
    /// @brief Configure CURVE authentication for a given domain.  CURVE 
    ///        authentication uses a directory (table) that holds all public
    ///        client certificates (i.e., public keys).
    /// @param[in] domain     The directory (table) containing all public client
    ///                       certificates.  All domains are covered with "*".
    /// @param[in] directory  The directory containing the public client
    ///                       certificates. 
    /// @note Certificates can be added or removed from the directory (table)
    ///       at any time.  However, this function must be called in order
    ///       to following the update so that the authenticator's state can
    ///       be changed to reflect the updated certificate information.
    void configureCurve(const std::string domain = "*",
                        const std::string directory = "");

    /// @result The minimum user privileges.
    [[nodiscard]] UserPrivileges getMinimumUserPrivileges() const;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Destructor.
    ~Service();
    /// @}

    Service(const Service &authenticator) = delete;
    Service& operator=(const Service &authenticator) = delete;
    Service(Service &&authenticator) noexcept = delete;
    Service& operator=(Service &&authenticator) noexcept = delete;
private:
    class ServiceImpl;
    std::unique_ptr<ServiceImpl> pImpl;
};
}
#endif
