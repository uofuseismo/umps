#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_PARAMETERS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_PARAMETERS_HPP
#include <memory>
#include "umps/logging/level.hpp"
namespace UMPS::Messaging::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Services::ConnectionInformation
{
/// @class Parameters "parameters.hpp" "umps/services/connectionInformation/parameters.hpp"
/// @brief The parameters for controlling the connection information service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Parameters
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Parameters();
    /// @brief Copy constructor.
    /// @param[in] parameters  The parameters from which to initialize
    ///                        this class. 
    Parameters(const Parameters &parameters);
    /// @brief Move constructor.
    /// @param[in] parameters  The parameters from which to initialize this
    ///                        class.  On exit, parameter's behavior is
    ///                        undefined. 
    Parameters(Parameters &&parameters) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment.
    /// @param[in] parameters   The parameters class to copy to this.
    /// @result A deep copy of parameters.
    Parameters& operator=(const Parameters &parameters);
    /// @brief Move assignment.
    /// @param[in,out] parameters  The parameters whose memory will be moved to
    ///                            this.  On exit, parameters's behavior is
    ///                            undefined.
    /// @result The memory from parameters moved to this.
    Parameters& operator=(Parameters &&parameters) noexcept;
    /// @}

    /// @brief Loads the parameters from an initialization file.
    /// @param[in] fileName   The name of the initialization file.
    /// @param[in] section    The section of the initialization file with the
    ///                       information to be parsed.  This will likely
    ///                       be of the form "Counter.Name" where Name is the
    ///                       name of the item to count e.g., "Pick",
    ///                       "Amplitude", etc.
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
        const UMPS::Messaging::Authentication::ZAPOptions &zapOptions) noexcept; 
    /// @result The ZAP options.
    [[nodiscard]] 
    UMPS::Messaging::Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @param[in] verbosity   The verbosity.
    void setVerbosity(UMPS::Logging::Level level) noexcept;
    /// @result The verbosity of the conter.
    [[nodiscard]] UMPS::Logging::Level getVerbosity() const noexcept;
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~Parameters();
    /// @}
private:
    class ParametersImpl;
    std::unique_ptr<ParametersImpl> pImpl;    
};
}
#endif
