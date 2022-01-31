#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_GETCONNECTIONS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_GETCONNECTIONS_HPP
#include <vector>
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Services::ConnectionInformation
{
class Details;
}
namespace UMPS::Services::ConnectionInformation
{
/// @brief Convenience function to request all the available connections.
/// @param[in] endPoint   The address of the operator's ConnectionDetails
///                       service.
/// @result The connection details.
/// @throws std::runtime_error if the connection fails.
/// @note This works for the grasslands or strawhouse security paradigms.
[[deprecated]] [[nodiscard]] std::vector<Details> getConnections(const std::string &endPoint);
/// @brief Convenience function to request all the available connections.
/// @param[in] endPoint    The address of the operator's ConnectionDetails
///                        service. 
/// @param[in] zapOptions  The appropriate ZAP options.  This would be used as 
///                        a client.  For the the woodhouse security method
///                        you would have to set your user name and password
///                        and for the stonehouse you would have to set your
///                        public and private key as well as the server
///                        private key.
/// @throws std::runtime_error if the connection fails.
[[deprecated]] [[nodiscard]]
std::vector<Details> getConnections(
    const std::string &endPoint,
    const UMPS::Authentication::ZAPOptions &zapOptions);
}
#endif
