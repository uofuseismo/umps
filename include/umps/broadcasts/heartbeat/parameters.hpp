#ifndef UMPS_BROADCASTS_HEARTBEAT_PARAMETERS_HPP
#define UMPS_BROADCASTS_HEARTBEAT_PARAMETERS_HPP
#include <memory>
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Broadcasts::Heartbeat
{
/// @class Parameters "parameters.hpp" "umps/broadcasts/heartbeat/parameters.hpp"
/// @brief Defines the parameters for the underlying proxy socket that enables
///        the packet broadcast.  Note the terminology - publishers connect
///        to the frontend and subscribers connect to the backend so that
///        data flows from front to back.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Parameters
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    Parameters();
    /// @brief Copy constructor.
    /// @param[in] parameters  The parameters class from which to initialize 
    ///                        this class.
    Parameters(const Parameters &parameters);
    /// @brief Move constructor.
    /// @param[in,out] parameters  The parameters class from which to initialize
    ///                            this class.  On exit, parameters's behavior
    ///                            is undefined.
    Parameters(Parameters &&parameters) noexcept;

    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] parameters  The parameters class to copy to this.
    /// @result A deep copy of the input parameters.
    Parameters& operator=(const Parameters &parameters);
    /// @brief Move assignment operator.
    /// @param[in,out] parameters  The parameters class whose memory will
    ///                            be moved to this.  On exit, parameters's
    ///                            behavior is undefined.
    /// @result The memory from parameters moved to this.
    Parameters& operator=(Parameters &&parameters) noexcept;

    /// @}

    /// @name Connection Addresses
    /// @{

    /// @brief Sets the frontend's IP address.
    /// @param[in] address  The address of the frontend.
    /// @throws std::invalid_argument if this is empty.
    void setFrontendAddress(const std::string &address);
    /// @result The frontend's IP address.
    /// @throws std::runtime_error if \c haveFrontendAddress() is false.
    [[nodiscard]] std::string getFrontendAddress() const; 
    /// @result True indicates the frontend address was set.
    [[nodiscard]] bool haveFrontendAddress() const noexcept;

    /// @brief Sets the backend's IP address.
    /// @param[in] address  The address of the backend.
    /// @throws std::invalid_argument if this is empty.
    void setBackendAddress(const std::string &address);
    /// @result The backend's IP address.
    /// @throws std::runtime_error if \c haveBackendAddress() is false.
    [[nodiscard]] std::string getBackendAddress() const;
    /// @result True indicates the backend address was set.
    [[nodiscard]] bool haveBackendAddress() const noexcept;

    /// @}  

    /// @name High-Water Mark
    /// @{
    /// @brief Sets the frontend's high water mark.
    /// @param[in] highWaterMark  The approximate number of messages that can
    ///                           be cached by the frontend.  Note, that setting
    ///                           this to 0 effectively makes it infinite.
    /// @throws std::invalid_argument if the high water mark is negative.
    void setFrontendHighWaterMark(const int highWaterMark);
    /// @result The high-water mark for the frontend.
    [[nodiscard]] int getFrontendHighWaterMark() const noexcept;

    /// @brief Sets the backend's high water mark.
    /// @param[in] highWaterMark  The approximate number of messages that can
    ///                           be cached by the backend.  Note, that setting
    ///                           this to 0 effectively makes it infinite.
    /// @throws std::invalid_argument if the high water mark is negative.
    void setBackendHighWaterMark(const int highWaterMark);
    /// @result The high-water mark for the backend.
    [[nodiscard]] int getBackendHighWaterMark() const noexcept;
    /// @}

    /// @name ZAP Options
    /// @{

    /// @brief Sets the ZeroMQ Authentication Protocol options.
    /// @param[in] zapOptions  The ZAP options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &zapOptions) noexcept; 
    /// @result The ZAP options.
    [[nodiscard]] UMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;

    /// @}
 
    /// @result The name of the broadcast.
    [[nodiscard]] static std::string getName() noexcept;

    /// @brief Loads parameters from an initialization file.
    /// @param[in] iniFile  The name of the initialization file.
    /// @param[in] section  The section of the ini file from which to
    ///                     read variables.
    void parseInitializationFile(const std::string &iniFile,
                                 const std::string &section = "Heartbeat");

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
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
