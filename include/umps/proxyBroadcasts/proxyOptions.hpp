#ifndef UMPS_PROXY_BROADCASTS_PROXY_OPTIONS_HPP
#define UMPS_PROXY_BROADCASTS_PROXY_OPTIONS_HPP
#include <memory>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
 namespace Messaging::XPublisherXSubscriber
 {
  class ProxyOptions;
 }
}
namespace UMPS::ProxyBroadcasts
{
/// @class ProxyOptions "proxyOptions.hpp" "umps/proxyBroadcasts/proxyOptions.hpp"
/// @brief Defines the options for the underlying proxy socket that enables
///        the packet cache service.  Note the terminology - clients connect
///        to the frontend and servers connect to the backend.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup UMPS_ProxyBroadcasts
class ProxyOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    ProxyOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    ProxyOptions(const ProxyOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    ProxyOptions(ProxyOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options.
    ProxyOptions& operator=(const ProxyOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will
    ///                         be moved to this.  On exit, options's
    ///                         behavior is undefined.
    /// @result The memory from options moved to this.
    ProxyOptions& operator=(ProxyOptions &&options) noexcept;
    /// @}

    /// @name Connection Addresses
    /// @{

    /// @brief Sets the frontend's IP address.
    /// @param[in] address  The address of the frontend.
    /// @throws std::invalid_argument if this is empty.
    void setFrontendAddress(const std::string &address);

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

    /// @name Name of the Proxy
    /// @{

    /// @brief Sets the name of the proxy.
    /// @param[in] name  The name of the proxy.
    /// @throws std::invalid_argument if the name is empty.
    void setName(const std::string &name);
    /// @result The name of the proxy broadcast.
    /// @throws std::invalid_argument if \c haveName() is false.      
    [[nodiscard]] std::string getName() const;
    /// @result True indicates the name was set.
    [[nodiscard]] bool haveName() const noexcept;
    /// @}

    /// @name High-Water Mark
    /// @{

    /// @brief Sets the frontend's high water mark.
    /// @param[in] highWaterMark  The approximate number of messages that can
    ///                           be cached by the frontend.  Note, that setting
    ///                           this to 0 effectively makes it infinite.
    /// @throws std::invalid_argument if the high water mark is negative.
    void setFrontendHighWaterMark(int highWaterMark);

    /// @brief Sets the backend's high water mark.
    /// @param[in] highWaterMark  The approximate number of messages that can
    ///                           be cached by the backend.  Note, that setting
    ///                           this to 0 effectively makes it infinite.
    /// @throws std::invalid_argument if the high water mark is negative.
    void setBackendHighWaterMark(int highWaterMark);
    /// @}

    /// @name ZAP Options
    /// @{

    /// @brief Sets the ZeroMQ Authentication Protocol options.
    /// @param[in] zapOptions  The ZAP options.
    void setZAPOptions(const UMPS::Authentication::ZAPOptions &zapOptions) noexcept;
    /// @}

    /// @result The router/dealer proxy options.
    [[nodiscard]] UMPS::Messaging::XPublisherXSubscriber::ProxyOptions getProxyOptions() const noexcept;
 
    /// @brief Loads proxy options from an initialization file.
    /// @param[in] iniFile  The name of the initialization file.
    /// @param[in] section  The section of the ini file from which to
    ///                     read variables.
    void parseInitializationFile(const std::string &iniFile,
                                 const std::string &section);
    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~ProxyOptions();
    /// @}
private:
    class ProxyOptionsImpl;
    std::unique_ptr<ProxyOptionsImpl> pImpl;
};
}
#endif
