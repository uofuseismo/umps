#ifndef UMPS_AUTHENTICATION_CERTIFICATE_ENUMS_HPP
#define UMPS_AUTHENTICATION_CERTIFICATE_ENUMS_HPP
namespace UMPS::Authentication::Certificate
{
/// @brief Influences the security of the hashed passwords and keys.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class HashLevel
{
    INTERACTIVE,  /*!< This is a fast and low-security hashing technique.
                       It requires about 64 Mb of RAM. */
    MODERATE,     /*!< This is an intermediate hashing technique. 
                       It requires about 256 Mb of RAM and takes about
                       0.7 seconds on a 2.8 Ghz Core i7 CPU. */
    SENSITIVE     /*!< This is for highly-sensitive data such as passwords.
                       It requires about 1024 Mb of RAM and takes about
                       3.5 seconds on a 2.8 Ghz Core i7 CPU. */
};
}
#endif
