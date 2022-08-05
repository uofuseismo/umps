#ifndef UMPS_PROXYSERVICES_INCREMENTER_REQUESTOROPTIONS_HPP
#define UMPS_PROXYSERVICES_INCREMENTER_REQUESTOROPTIONS_HPP
#include <memory>
#include <chrono>
// Forward declarations
namespace UMPS
{
 namespace MessageFormats
 {
  class Messages;
  class IMessage;
 }
 namespace Messaging::RouterDealer
 {
  class RequestOptions;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::ProxyServices::Incrementer
{
/// @class RequestorOptions "requestorOptions.hpp" "umps/proxyServices/incrementer/requestorOptions.hpp"
/// @brief Defines the options for the incrementer requestor.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RequestorOptions
{
public:
    /// @name Constructor
    /// @{

    /// @brief Constructor.
    RequestorOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    RequestorOptions(const RequestorOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    RequestorOptions(RequestorOptions &&options) noexcept;

    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of options.
    RequestorOptions& operator=(const RequestorOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    RequestorOptions& operator=(RequestorOptions &&options) noexcept;

    /// @}

    /// @name Address 
    /// @{

    /// @brief Sets the address of the router to which requests will be
    ///        submitted and from which replies will be received.
    /// @param[in] address  The address of the router.
    /// @throws std::invalid_argument if address is blank.
    void setAddress(const std::string &address);
    /// @result The address to of the router.
    /// @throws std::runtime_error if \c haveAddress() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @result True indicates that the address was set.
    [[nodiscard]] bool haveAddress() const noexcept;
    /// @}

    /// @name High Water Mark
    /// @{

    /// @param[in] highWaterMark  The approximate max number of messages to 
    ///                           cache on the socket.  0 will set this to
    ///                           "infinite".
    /// @throws std::invalid_argument if highWaterMark is negative.
    void setHighWaterMark(int highWaterMark);
    /// @result The high water mark.  The default is 0.
    [[nodiscard]] int getHighWaterMark() const noexcept;
    /// @}

    /// @name ZeroMQ Authentication Protocol
    /// @{

    /// @brief Sets the ZAP options.
    /// @param[in] options  The ZAP options which will define the socket's
    ///                     security protocol.
    void setZAPOptions(const Authentication::ZAPOptions &options);
    /// @result The ZAP options.
    [[nodiscard]] Authentication::ZAPOptions getZAPOptions() const noexcept;
    /// @}

    /// @result The request options.
    [[nodiscard]] UMPS::Messaging::RouterDealer::RequestOptions
        getRequestOptions() const noexcept;

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RequestorOptions();
    /// @}
private:
    class RequestorOptionsImpl;
    std::unique_ptr<RequestorOptionsImpl> pImpl;
};
}
#endif