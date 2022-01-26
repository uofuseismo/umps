#ifndef UMPS_PROXYSERVICES_DATAPACKET_PROXYOPTIONS_HPP
#define UMPS_PROXYSERVICES_DATAPACKET_PROXYOPTIONS_HPP
#include <memory>
namespace UMPS::Authentication
{
 class ZAPOptions;
}
namespace UMPS::ProxyServices::PacketCache
{
/// @class ProxyOptions "proxyOptions.hpp" "umps/proxyServices/packetCache/proxyOptions.hpp"
/// @brief Defines the options for the underlying proxy socket that enables
///        the packet cache service.  Note the terminology - clients connect
///        to the frontend and servers connect to the backend.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
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
 
    /// @name Name
    /// @{

    /// @brief Sets the name of the packet cache proxy service.
    /// @param[in] name  The name to set.
    /// @throws std::invalid_argument if name is empty.
    void setName(const std::string &name);
    /// @result The name of the proxy service.
    /// @throws std::runtime_error if the name was not set.
    [[nodiscard]] std::string getName() const;
    /// @result True indicates the name was set.
    [[nodiscard]] bool haveName() const noexcept;
    /// @}

    /// @brief Loads proxy options from an initialization file.
    /// @param[in] iniFile  The name of the initialization file.
    /// @param[in] section  The section of the ini file from which to
    ///                     read variables.
    void parseInitializationFile(const std::string &iniFile,
                                 const std::string &section = "DataPacket");
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
