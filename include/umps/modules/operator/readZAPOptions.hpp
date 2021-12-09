#ifndef UMPS_MODULES_OPERATOR_READZAPOPTIONS_HPP
#define UMPS_MODULES_OPERATOR_READZAPOPTIONS_HPP
#include <string>
#include "umps/messaging/authentication/zapOptions.hpp"
namespace boost::program_options
{
class ptree;
}
namespace UMPS::Modules::Operator
{
/// @brief Parses an ini file and loads the uOperator ZAP security options for
///        a ZAP client.
/// @param[in] iniFile  The name of the initialization file to parse.
/// @result The corresponding ZAP options.
/// @throws std::invalid_argument if the iniFile does not exist.
/// @throws std::runtime_error if the options specified in the ini file are
///         invalid.
/// @note By default a grasslands client will be returned.
UMPS::Messaging::Authentication::ZAPOptions
    readZAPClientOptions(const std::string &iniFile);
/// @brief Parses an ini file and loads the uOperator ZAP security options for
///        a ZAP client.
/// @param[in] propertyTree  The boost property tree.
/// @result The corresponding ZAP options.
/// @throws std::runtime_error if the options specified in the ini file are
///         invalid.
/// @note By default a grasslands client will be returned.
UMPS::Messaging::Authentication::ZAPOptions 
   readZAPClientOptions(const boost::property_tree::ptree &propertyTree);

/// @brief Parses an ini file and loads the uOperator ZAP security options for
///        a ZAP server.
/// @param[in] iniFile  The name of the initialization file to parse.
/// @result The corresponding ZAP options.
/// @throws std::invalid_argument if the iniFile does not exist.
/// @throws std::runtime_error if the options specified in the ini file are
///         invalid.
/// @note By default a grasslands server will be returned.
UMPS::Messaging::Authentication::ZAPOptions
    readZAPServerOptions(const std::string &iniFile);
/// @brief Parses an ini file and loads the uOperator ZAP security options for
///        a ZAP server.
/// @param[in] propertyTree  The boost property tree.
/// @result The corresponding ZAP options.
/// @throws std::runtime_error if the options specified in the ini file are
///         invalid.
/// @note By default a grasslands server will be returned.
UMPS::Messaging::Authentication::ZAPOptions 
   readZAPServerOptions(const boost::property_tree::ptree &propertyTree);
}
#endif
