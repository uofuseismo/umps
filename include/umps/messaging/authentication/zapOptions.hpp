#ifndef UMPS_MESSAGING_AUTHENTICATION_ZAPOPTIONS_HPP
#define UMPS_MESSAGING_AUTHENTICATION_ZAPOPTIONS_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
// Forward declarations
namespace zmq
{
 class socket_t;
}
namespace UMPS::Messaging::Authentication::Certificate
{
 class UserNameAndPassword;
 class Keys;
}
namespace UMPS::Messaging::Authentication
{
/// @class ZAPOptions "zapOptions.hpp" "umps/messaging/authentication/zapOptions.hpp"
/// @brief Defines options for using the ZeroMQ authentication protocol.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class ZAPOptions
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    ZAPOptions();
    /// @brief Copy assignment operator.
    /// @param[in] options  The options class from which to initialize
    ///                     this class. 
    ZAPOptions(const ZAPOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior is
    ///                         undefined.
    ZAPOptions(ZAPOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] options   The options to copy to this.
    /// @result A deep copy of the input options.
    ZAPOptions& operator=(const ZAPOptions &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    ZAPOptions& operator=(ZAPOptions &&options) noexcept;
    /// @}

    /// @name Grasslands
    /// @{
    /// @brief This enables the grasslands security pattern.  Effectively, there
    ///        is no security.  This is the default security pattern.
    void setGrasslandsClient() noexcept;
    /// @brief This enables the grasslands security pattern.  Effectively, there
    ///        is no security.  This is the default security pattern.
    void setGrasslandsServer() noexcept;
    /// @}
    
    /// @name Strawhouse
    /// @{
    /// @brief This enables the strawhouse security pattern for the client.
    ///        In this case nothing else needs to be specified.
    void setStrawhouseClient() noexcept;
    /// @brief This enables the strawhouse security pattern for a ZAP server.
    ///        The server will be provided the client's IP address on 
    ///        \c getDomain() and an auxiliary service will verify.
    void setStrawhouseServer() noexcept;
    /// @}

    /// @name Woodhouse
    /// @{
    /// @brief This enables the woodhouse security pattern for the client.
    ///        Here, the client provides their username and password which
    ///        ZeroMQ will pass to the server during authentication.
    ///        Additionally, the IP may be checked.
    /// @param[in] credentials  The client's name and corresponding password.
    /// @throws std::invalid_argument if credentials.haveName() or 
    ///         credentials.havePassword() is false.
    void setWoodhouseClient(const Certificate::UserNameAndPassword &credentials);
    /// @brief This enables the woodhouse security pattern for the server.
    ///        The server will be provided the client's name, password, and IP
    ///        on \c getDomain() then an auxiliary service will verify.
    void setWoodhouseServer() noexcept;
    /// @result The client's username and password.
    [[nodiscard]] Certificate::UserNameAndPassword getClientCredentials() const;
    /// @}

    /// @name Stonehouse
    /// @{
    /// @brief This enables the stonehouse security pattern for the client.
    ///        The client must know the server's public key.  Additionally,
    ///        the client must have a valid public and private key pair.
    ///        The public key will be encrypted, sent to the server, and
    ///        validated.  Additionally, the IP may be validated.
    /// @param[in] serverKey  The server's public key.
    /// @param[in] clientKey  The client's public and private key pair.
    /// @throws std::invalid_argument if server.havePublicKey() is false,
    ///         client.havePublicKey() is false, or client.havePrivateKey()
    ///         is false. 
    /// @note Upon successful completion, isAuthenticationServer() will
    ///       be false.
    void setStonehouseClient(const Certificate::Keys &serverKeys,
                             const Certificate::Keys &clientKeys); 
    /// @brief This enbales the stonehouse security pattern for the server.
    ///        The server will be provided the client's public key and IP
    ///        on \c getDomain() then an auxiliary service will verify.
    /// @param[in] serverKeys  The server's public and private keys.
    /// @throws std::invalid_argument if server.havePublicKey() or 
    ///         \c server.havePrivateKey() is false.
    void setStonehouseServer(const Certificate::Keys &serverKeys);
    /// @result The server's public and, potentially, private key information.
    [[nodiscard]] Certificate::Keys getServerKeys() const;
    /// @result The client's public and, potentially, private key information. 
    [[nodiscard]] Certificate::Keys getClientKeys() const;
    /// @}

    /// @name ZAP Domain
    /// @{
    /// @brief Sets the ZeroMQ Authentication Protocol Domain.  Effectively,
    ///        ZeroMQ will send an inter-process message to the authenticator
    ///        on a connection that utilizes this name.
    /// @param[in] zapDomain  The ZAP domain.
    /// @throws std::invalid_argument if zapDomain is blank.
    /// @note This will only be accessed if \c isAuthenticationServer() is true.
    void setDomain(const std::string &domain);
    /// @result The ZAP domain.  By default this is "global".
    [[nodiscard]] std::string getDomain() const noexcept;
    /// @}

    /// @name Auxiliary Properties
    /// @{
    /// @result The currently set security level.
    /// @note This is automatically set by the most recent call to the
    ///       \c setGrasslandsClient(), \c setGrasslandsServer(),
    ///       \c setStrawhouseClient(), \c setStrawhouseServer(),
    ///       \c setWoodhouseClient(), \c setWoodhouseServer(),
    ///       \c setStonehouseClient(), \c setStonehouseServer().
    [[nodiscard]] SecurityLevel getSecurityLevel() const noexcept;
    /// @result True indicates this is a ZAP authentication server.
    ///         False indicates this is a ZAP authentication client.
    /// @note This is automatically set by the most recent call to the
    ///       \c setGrasslandsClient(), \c setGrasslandsServer(),
    ///       \c setStrawhouseClient(), \c setStrawhouseServer(),
    ///       \c setWoodhouseClient(), \c setWoodhouseServer(),
    ///       \c setStonehouseClient(), \c setStonehouseServer().
    [[nodiscard]] bool isAuthenticationServer() const noexcept;
    /// @}

    /// @brief A utility routine that sets the ZAP options on the
    ///        ZeroMQ socket.
    /// @param[in,out] socket  The ZeroMQ socket.  On input, this socket must
    ///                        not be connected when this is called, i.e.,
    ///                        use this method before a zmq_bind or zmq_connect 
    ///                        call.  On exit, the appropriate ZeroMQ ZAP
    ///                        options for the desired security level will be
    ///                        set on the socket.
    /// @throws std::runtime_error if the options cannot be set.
    void setSocketOptions(zmq::socket_t *socket) const;

    /// @name Destructors
    /// @{
    /// @brief Resets the class to the grasslands pattern and releases
    ///        all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~ZAPOptions();
    /// @}
private:
    class ZAPOptionsImpl; 
    std::unique_ptr<ZAPOptionsImpl> pImpl;
};
}
#endif
