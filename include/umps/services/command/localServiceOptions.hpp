#ifndef UMPS_SERVICES_COMMAND_LOCALSERVICEOPTIONS_HPP
#define UMPS_SERVICES_COMMAND_LOCALSERVICEOPTIONS_HPP
#include <memory>
#include <chrono>
#include <functional>
#include <string>
namespace UMPS
{
 namespace MessageFormats
 {
  class IMessage;
 }
}
namespace UMPS::Services::Command
{
/// @class LocalServiceOptions "localServiceOptions.hpp" "umps/services/command/localServiceOptions.hpp"
/// @brief This class allows a background application to interact with a user.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class LocalServiceOptions
{
public:
    /// @name Constructors
    /// @{
 
    /// @brief Constructor.
    LocalServiceOptions();
    /// @brief Copy constructor.
    /// @param[in] options  The options class from which to initialize
    ///                     this class.
    LocalServiceOptions(const LocalServiceOptions &options);
    /// @brief Move constructor.
    /// @param[in,out] options  The options class from which to initialize this
    ///                         class.  On exit, options's behavior is
    ///                         undefined.
    LocalServiceOptions(LocalServiceOptions &&options) noexcept;
    /// @}

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] options  The options to copy to this.
    /// @result A deep copy of the input options.
    LocalServiceOptions& operator=(const LocalServiceOptions &options);
    /// @brief Move assignment.
    /// @param[in] options  The options class whose memory will be moved
    ///                     to this.  On exit, options's behavior is undefined.
    /// @result The memory from the options moved to this.
    LocalServiceOptions& operator=(LocalServiceOptions &&options) noexcept;
    /// @}

    /// @name Module Name (Required)
    /// @{
 
    /// @brief Sets the name of themodule.
    /// @param[in] moduleName  The name of the module.
    void setModuleName(const std::string &moduleName);
    /// @result The module name.
    /// @throws std::runtime_error if \c haveModuleName() is false.
    [[nodiscard]] std::string getModuleName() const;
    /// @result True indicates the module name was set.
    [[nodiscard]] bool haveModuleName() const noexcept;
    /// @result The address of the connection.
    /// @throws std::runtime_error if \c haveModuleName() is false.
    [[nodiscard]] std::string getAddress() const;
    /// @}

    /// @name Callback (Required)
    /// @{

    /// @brief The callback function that defines how the local command service
    ///        will reply to commands.
    /// @param[in] callback  The callback function which processes the
    ///                      command request message.
    /// @note It is important this function never throw, crash, etc.  This is
    ///       when exiting gracefully matters.
    void setCallback(const std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
                                         (const std::string &messageType, const void *data, size_t length)> &callback);
    /// @result The callback function.
    /// @throws std::invalid_argument if \c haveCallback() is false.
    [[nodiscard]]
    std::function<std::unique_ptr<UMPS::MessageFormats::IMessage>
        (const std::string &messageType, const void *data, size_t length)> getCallback() const;
    /// @result True indicates the callback was set.
    [[nodiscard]] bool haveCallback() const noexcept;
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
    /// @}

    /// @name IPC Directory
    /// @{

    /// @brief Sets the directory where IPC communication will occur.
    /// @param[in] directory  The IPC directory. 
    void setIPCDirectory(const std::string &directory);
    /// @result The directory where IPC communication will take place.
    [[nodiscard]] std::string getIPCDirectory() const noexcept;
    /// @}

    /// @name Module Table Directory
    /// @{

    /// @brief Sets the sqlite3 file name that will hold the currently running
    ///        local module information.
    /// @param[in] fileName  The name of the sqlite3 file with the local_module
    ///                      table.
    /// @throws std::runtime_error if the directory to contain the file cannot
    ///         be made.
    void setLocalModuleTable(const std::string &fileName);
    /// @result The sqlite3 file that contains the local_modules table.
    [[nodiscard]] std::string getLocalModuleTable() const noexcept;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases memory.
    void clear() noexcept;
    /// @brief Destructor
    ~LocalServiceOptions();
    /// @} 
private:
    class LocalServiceOptionsImpl;
    std::unique_ptr<LocalServiceOptionsImpl> pImpl;
};
}
#endif
