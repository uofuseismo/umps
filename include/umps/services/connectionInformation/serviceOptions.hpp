#ifndef UMPS_SERVICES_CONNECTION_INFORMATION_SERVICE_OPTIONS_HPP
#define UMPS_SERVICES_CONNECTION_INFORMATION_SERVICE_OPTIONS_HPP
#include <memory>
#include "umps/logging/level.hpp"
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Services::ConnectionInformation
{
/// @class ServiceOptions "serviceOptions.hpp" "umps/services/connectionInformation/serviceOptions.hpp"
/// @brief The options for controlling the connection information service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Applications_Operator
class ServiceOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ServiceOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options from which to initialize
    ///                     this class. 
    ServiceOptions(const ServiceOptions &options);
    /// @brief Move constructor.
    /// @param[in] options  The options from which to initialize this
    ///                     class.  On exit, parameter's behavior is
    ///                     undefined. 
    ServiceOptions(ServiceOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options   The options class to copy to this.
    /// @result A deep copy of options.
    ServiceOptions& operator=(const ServiceOptions &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options whose memory will be moved to
    ///                         this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    ServiceOptions& operator=(ServiceOptions &&options) noexcept;
    /// @}

    /// @brief Loads the options from an initialization file.
    /// @param[in] fileName   The name of the initialization file.
    /// @param[in] section    The section of the initialization file with the
    ///                       information to be parsed.
    /// @throws std::invalid_argument if the initialization file does not,
    ///         exist cannot be parsed, does not have the specified section,
    ///         or has incorrect information.
    void parseInitializationFile(const std::string &fileName,
                                 const std::string &section);

    /// @name Required Parameters
    /// @{

    /// @result The name of the connectionInformation service.
    [[nodiscard]] static std::string getName() noexcept;

    /// @brief Sets the proxy's address to which the clients will connect.
    /// @param[in] address  The address from which clients will connect to this
    ///                     service.
    void setClientAccessAddress(const std::string &address);
    /// @result The address from which clients will access the service.
    /// @throws std::runtime_error if \c haveClientAccessAddress() is false.
    [[nodiscard]] std::string getClientAccessAddress() const; 
    /// @result True indicates that the client access address was set.
    [[nodiscard]] bool haveClientAccessAddress() const noexcept;
    /// @}

    /// @name Optional Parameters
    /// @{

    /// @brief Sets the ZeroMQ Authentication Protocol options.
    /// @param[in] zapOptions  The ZAP options.
    void setZAPOptions(
        const UMPS::Authentication::ZAPOptions &zapOptions) noexcept; 
    /// @result The ZAP options.
    [[nodiscard]] 
    UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @param[in] level   The logging level.
    void setVerbosity(UMPS::Logging::Level level) noexcept;
    /// @result The verbosity of the conter.
    [[nodiscard]] UMPS::Logging::Level getVerbosity() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~ServiceOptions();
    /// @}
private:
    class ServiceOptionsImpl;
    std::unique_ptr<ServiceOptionsImpl> pImpl;    
};
}
#endif
