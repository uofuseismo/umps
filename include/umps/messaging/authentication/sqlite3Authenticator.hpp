#ifndef UMPS_MESSAGING_AUTHENTICATION_SQLITE3AUTHENTICATOR_HPP
#define UMPS_MESSAGING_AUTHENTICATION_SQLITE3AUTHENTICATOR_HPP
#include <memory>
#include "umps/messaging/authentication/authenticator.hpp"
/*
namespace zmq 
{
 class context_t;
 class socket_t;
}
*/
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Messaging::Authentication
{
 class User;
 namespace Certificate
 {
  class Keys;
  class UserNameAndPassword;
 }
}
namespace UMPS::Messaging::Authentication
{
/// @class SQLite3Authenticator "sqlite3Authenticator.hpp" "umps/messaging/authentication/sqlite3Authenticator.hpp"
/// @brief Performs user authentication against a SQLite3 database.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class SQLite3Authenticator : public IAuthenticator
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    SQLite3Authenticator();
    /// @brief Move constructor.
    /// @param[in] authenticator  The authenticator from which to initialize
    ///                           this class.  On exit, authenticator's behavior
    ///                           is undefined.
    SQLite3Authenticator(SQLite3Authenticator &&authenticator) noexcept;
    /// @brief Constructor with a specified logger.
    /// @param[in] logger   The logging utility.
    explicit SQLite3Authenticator(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] authentciator  The authenticator class whose memory will
    ///                               be moved to this.  On exit,
    ///                               authenticator's behavior is undefined.
    /// @result The memory from authenticator moved to this.
    SQLite3Authenticator& operator=(SQLite3Authenticator &&authenticator) noexcept;
    /// @}

    /// @name Tables
    /// @{
    /// @brief Opens the users table.
    /// @param[in] fileName   The name of the SQLite3 user table file.
    /// @param[in] createIfDoesNotExist  If true and the table does not exist
    ///                                  then the application will attempt to
    ///                                  create the table.
    void openUsersTable(const std::string &fileName,
                        bool createIfDoesNotExist = true);
    /// @result True indicates that the user's table is opened.
    [[nodiscard]] bool haveUsersTable() const noexcept;

    /// @brief Opens the blacklist table.
    /// @param[in] fileName   The name of the SQLite3 blacklist table file.
    /// @param[in] createIfDoesNotExist  If true and the table does not exist
    ///                                  then the application will attempt to
    ///                                  create the table.
    void openBlacklistTable(const std::string &fileName,
                            bool createIfDoesNotExist = true);  
    /// @result True indicates that the blacklist table is opened.
    [[nodiscard]] bool haveBlacklistTable() const noexcept;

    /// @brief Opens the whitelist table.
    /// @param[in] fileName   The name of the SQLite3 blacklist table file.
    /// @param[in] createIfDoesNotExist  If true and the table does not exist
    ///                                  then the application will attempt to
    ///                                  create the table.
    void openWhitelistTable(const std::string &fileName,
                            bool createIfDoesNotExist = true);
    /// @result True indicates that the whitelist table is opened.
    [[nodiscard]] bool haveWhitelistTable() const noexcept;
    /// @}

    /// @name White and Blacklisting
    /// @{
    /// @brief Denies access to a certain IP address.
    /// @param[in] address  The address to add to the blacklist.
    /// @throws std::invalid_argument if the address is whitelisted or empty.
    void addToBlacklist(const std::string &address);
    /// @brief Removes an IP address from the blacklist.
    /// @param[in] address  The address to remove from the blacklist.
    void removeFromBlacklist(const std::string &address) noexcept;
    /// @result True indicates the address is blacklisted.
    [[nodiscard]] ValidationResult isBlacklisted(const std::string &address) const noexcept override final;

    /// @brief Grants access to a certain IP address.
    /// @param[in] address  The address to add to the whitelist.
    /// @throws std::invalid_argument if the address is blacklisted or empty.
    void addToWhitelist(const std::string &address);
    /// @brief Removes an IP address from the whitelist.
    /// @param[in] address  The address to remove from the whitelist.
    void removeFromWhitelist(const std::string &address) noexcept;
    /// @result True indicates the address is whitelisted.
    [[nodiscard]] virtual ValidationResult isWhitelisted(const std::string &address) const noexcept override final;
    /// @}

    [[nodiscard]] ValidationResult isValid(
        const Certificate::UserNameAndPassword &credentials) const noexcept override final;
    /// @brief Validates a provided public key.
    [[nodiscard]] ValidationResult isValid(
        const Certificate::Keys &keys) const noexcept override final;

    /// @brief Creates and binds the ZAP socket.
    //void start();

    /// @brief Closes the ZAP socket.
    //void stop();

    /// @name Destructor
    /// @{
    /// @brief Destructor.
    virtual ~SQLite3Authenticator();
    /// @}

//    Authenticator(const Authenticator &authenticator) = delete;
//    Authenticator& operator=(const Authenticator &authenticator) = delete;
private:
    class AuthenticatorImpl;
    std::unique_ptr<AuthenticatorImpl> pImpl;
};
}
#endif
