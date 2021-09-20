#ifndef URTS_MESSAGING_AUTHENTICATION_GENERATEKEYS_HPP
#define URTS_MESSAGING_AUTHENTICATION_GENERATEKEYS_HPP
#include <vector>
namespace URTS::Messaging::Authentication
{
/// @brief Uses libsodium to generate a public/private key pair.  The public
///        key must be installed on the server running the URTS operator.
/// @result result.first is the public key and result.second is the private key.
/// @throws std::runtime_error in the case of an algorithmic failure.
[[nodiscard]]
std::pair<std::vector<char>, std::vector<char>> generateKeyPair();
}
#endif
