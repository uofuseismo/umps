#ifndef UMPS_SERVICES_COMMAND_REMOTEPROXYOPTIONS_HPP
#define UMPS_SERVICES_COMMAND_REMOTEPROXYOPTIONS_HPP
#include <memory>
namespace UMPS
{
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace UMPS::Services::Command
{
class RemoteProxyOptions
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    RemoteProxyOptions();
    /// @brief Copy constructor.
    RemoteProxyOptions(const RemoteProxyOptions &options);
    /// @brief Move constructor.
    RemoteProxyOptions(RemoteProxyOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options  The options class to copy to this.
    /// @result A deep copy of the input options.
    RemoteProxyOptions& operator=(const RemoteProxyOptions &options);
    /// @brief Move assignment.
    /// @param[in,out] options  The options class whose memory will be moved to
    ///                         this.  On exit, options's behavior is undefined.
    /// @result The memory from options moved to this.
    RemoteProxyOptions& operator=(RemoteProxyOptions &&options) noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~RemoteProxyOptions();
    /// @}
private:
    class RemoteProxyOptionsImpl;
    std::unique_ptr<RemoteProxyOptionsImpl> pImpl;
};
}
#endif
