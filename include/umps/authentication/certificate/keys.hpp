#ifndef UMPS_AUTHENTICATION_CERTIFICATE_KEYS_HPP
#define UMPS_AUTHENTICATION_CERTIFICATE_KEYS_HPP
#include <memory>
#include <array>
namespace UMPS::Authentication::Certificate
{
/// @class Keys "keys.hpp" "umps/authentication/certificate/keys.hpp"
/// @brief A class for managing a public/private key-based certificates
///        authenticating a client or server with ZeroMQ and libsodium.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Keys
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Keys();
    /// @brief Copy constructor.
    /// @param[in] keys  The certificate from which to initialize this class.
    Keys(const Keys &keys);
    /// @brief Move constructor.
    /// @param[in,out] keys  The certificate from which to initialize
    ///                      this class.  On exit, keys's behavior
    ///                      is undefined.
    Keys(Keys &&keys) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] keys  The certificate to copy to this.
    /// @result A deep copy of the input certificate.
    Keys& operator=(const Keys &keys);
    /// @brief Move assignment operator.
    /// @param[in,out] keys  The certificate whose memory will be moved
    ///                      to this.  On exit, keys's behavior
    ///                      is undefined.
    /// @result The memory moved from certificate to this.
    Keys& operator=(Keys &&keys) noexcept;
    /// @}

    /// @name Keypair Creation
    /// @{

    /// @brief This will create a new public/private keypair.
    /// @throws std::runtime_error if an algorithmic error occurred.
    void create();
    /// @}

    /// @name Keys
    /// @{

    /// @brief Sets the binary public key.
    /// @param[in] publicKey  The binary public key to set.
    /// @throws std::runtime_error in the case of algorithmic failure.
    void setPublicKey(const std::array<uint8_t, 32> &publicKey);
    /// @brief Sets the human-readable public key.
    /// @param[in] publicKeyText  The human-readable public key to set.
    /// @throws std::runtime_error in the case of algorithmic failure.
    void setPublicKey(const std::array<char, 41> &publicKeyText);
    /// @result The public key in a binary format.
    /// @throws std::runtime_error if \c havePublicKey() is false.
    [[nodiscard]] std::array<uint8_t, 32> getPublicKey() const;
    /// @result The public key in human readable format.
    /// @throws std::runtime_error if \c havePublicKey() is false.
    [[nodiscard]] std::array<char, 41> getPublicTextKey() const;
    /// @result True indicates that the public key was set.
    [[nodiscard]] bool havePublicKey() const noexcept;

    /// @brief Sets the binary private key.
    /// @param[in] privateKey  The binary private key to set.
    /// @throws std::runtime_error in the case of algorithmic failure.
    void setPrivateKey(const std::array<uint8_t, 32> &privateKey);
    /// @brief Sets the human-readable private key.
    /// @param[in] privateKeyText  The human-readable private key to set.
    /// @throws std::runtime_error in the case of algorithmic failure.
    void setPrivateKey(const std::array<char, 41> &privateKeyText);
    /// @result The private key in a binary format.
    /// @throws std::runtime_error if \c havePrivateKey() is false.
    [[nodiscard]] std::array<uint8_t, 32> getPrivateKey() const;
    /// @result The private key in human readable format.
    /// @throws std::runtime_error if \c havePrivateKey() is false.
    [[nodiscard]] std::array<char, 41> getPrivateTextKey() const;
    /// @result True indicates that the private key was set.
    [[nodiscard]] bool havePrivateKey() const noexcept;

    /// @brief Sets the keypair from the binary public and private key.
    /// @param[in] publicKey   The binary public key.
    /// @param[in] privateKey  The binary private key.
    /// @throws std::runtime_error in the case of algorithm failure.
    void setPair(const std::array<uint8_t, 32> &publicKey,
                 const std::array<uint8_t, 32> &privateKey);
    /// @brief Sets the keypair from a human-readable public and private key.
    /// @param[in] publicKeyText   The human readable public key.
    /// @param[in] privateKeyText  The human readable private key.
    /// @throws std::runtime_error in the case of algorithmic failure.
    void setPair(const std::array<char, 41> &publicKeyText,
                 const std::array<char, 41> &privateKeyText);
    /// @result Convenience function that indicates the public and private key
    ///         pair are set.
    [[nodiscard]] bool haveKeyPair() const noexcept;
    /// @}

    /// @name Metadata
    /// @{

    /// @brief Sets the metadata for this key pair.  This can be a description
    ///        like "Client test certificate"
    /// @param[in] metadata   The metadata for this keypair.
    void setMetadata(const std::string &metadata) noexcept;
    /// @result The metadata associated with this key pair.
    [[nodiscard]] std::string getMetadata() const noexcept;
    /// @}

    /// @name File Input/Output
    /// @{

    /// @brief Writes the public key to a text file.
    /// @param[in] fileName  The public key file name.  The recommended suffix
    ///                      is ".public_key".
    /// @throws std::runtime_error if \c havePublicKey() is false.
    void writePublicKeyToTextFile(const std::string &fileName) const;
    /// @brief Writes the private key to a text file.
    /// @param[in] fileName  The public key file name.  The recommended suffix
    ///                      is ".private_key".
    /// @throws std::runtime_error if \c havePrivateKey() is false.
    void writePrivateKeyToTextFile(const std::string &fileName) const;
    /// @brief Reads the public and/or private key from a text file.
    /// @param[in] fileName   The name of the file with the key(s).
    /// @throws std::invalid_argument if the file does not exist.
    /// @throws std::runtime_error if there is a formatting error.
    void loadFromTextFile(const std::string &fileName);
    /// @}


    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~Keys();
    /// @}
private:
    class KeysImpl;
    std::unique_ptr<KeysImpl> pImpl; 
};
}
#endif
