#ifndef UMPS_MODULES_OPERATOR_READZAPOPTIONS_HPP
#define UMPS_MODULES_OPERATOR_READZAPOPTIONS_HPP
#include <string>
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Modules::Operator
{
/// @brief Parses an ini file and loads the uOperator ZAP security options for
///        a ZAP client.
/// @param[in] iniFile  The name of the initialization file to parse.
/// @param[in] section  The section of the ini file from which to read.
/// @result The corresponding ZAP options.
/// @throws std::invalid_argument if the iniFile does not exist.
/// @throws std::runtime_error if the options specified in the ini file are
///         invalid.
/// @note By default a grasslands client will be returned.
UMPS::Authentication::ZAPOptions
    readZAPClientOptions(const std::string &iniFile,
                         const std::string &section = "uOperator");
/// @brief Parses an ini file and loads the uOperator ZAP security options for
///        a ZAP server.
/// @param[in] iniFile  The name of the initialization file to parse.
/// @param[in] section  The section of the ini file from which to read.
/// @result The corresponding ZAP options.
/// @throws std::invalid_argument if the iniFile does not exist.
/// @throws std::runtime_error if the options specified in the ini file are
///         invalid.
/// @note By default a grasslands server will be returned.
UMPS::Authentication::ZAPOptions
    readZAPServerOptions(const std::string &iniFile,
                         const std::string &section = "uOperator");
}
#endif
