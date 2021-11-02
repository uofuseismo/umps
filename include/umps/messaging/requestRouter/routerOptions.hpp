#ifndef UMPS_MESSAGING_REQUESTROUTER_ROUTEROPTIONS_HPP
#define UMPS_MESSAGING_REQUESTROUTER_ROUTEROPTIONS_HPP
#include <memory>
#include "umps/messaging/authentication/enums.hpp"
// Forward declarations
namespace UMPS::Messaging::Authentication
{
 class ZAPOptions;
}
namespace UMPS::Messaging::RequestRouter
{
/// @class RouterOptions "routerOptions.hpp" "umps/messaging/requestRouter/routerOptions.hpp"
/// @brief Defines the router socket options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class RouterOptions
{
public:
    /// @name Constructor
    /// @{
    /// @brief Constructor.
    RouterOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    RouterOptions(const RouterOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize
    ///                         this class.  On exit, options's behavior
    ///                         is undefined.
    RouterOptions(RouterOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of options.
    RouterOptions& operator=(const RouterOptions &options);
    /// @brief Move assignment operator.
    /// @param[in,out] options  The options class whose memory will be moved
    ///                         to this.  On exit, options's behavior is
    ///                         undefined.
    /// @result The memory from options moved to this.
    RouterOptions& operator=(RouterOptions &&options) noexcept;
    /// @}

    /// @name End Point
    /// @{
    /// @param[in] endPoint  The endpoint on which to accept connections.
    ///                      For example, "tcp://127.0.0.1:5555".
    /// @throws std::invalid_argument if endPoint is blank.
    void setEndPoint(const std::string &endPoint);
    /// @result The endpoint to which clients will connect.
    /// @throws std::runtime_error if \c haveEndPoint() is false.
    [[nodiscard]] std::string getEndPoint() const;
    /// @result True indicates that the end point was set.
    [[nodiscard]] bool haveEndPoint() const noexcept;
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

    /// @name Poll Time Out
    /// @{
    /// @brief When the service starts a thread will monitor messages on the
    ///        endpoint by "polling."  This controls how long the thread will
    ///        wait.  
    /// @param[in] timeOutInMilliSeconds  The timeout in milliseconds.  If this
    ///                                   is negative then the thread will wait
    ///                                   forever for a message (dangerous).
    ///                                   If this is 0 then thread will return
    ///                                   immediately (this is a good way to get
    ///                                   a thread to report as 100% in use in
    ///                                   a system monitor).
    void setPollTimeOut(const std::chrono::milliseconds &timeOutInMilliSeconds) noexcept;
    /// @result The amount of time to wait before timing out in the polling
    ///         operation.
    [[nodiscard]] std::chrono::milliseconds getPollTimeOut() const noexcept;
    /// @name Destructors
    /// @{
    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~RouterOptions();
    /// @}
private:
    class RouterOptionsImpl;
    std::unique_ptr<RouterOptionsImpl> pImpl;
};
}
#endif
