#ifndef UMPS_PROXY_SERVICES_COMMAND_AVAILABLE_MODULES_RESPONSE_HPP
#define UMPS_PROXY_SERVICES_COMMAND_AVAILABLE_MODULES_RESPONSE_HPP
#include <memory>
#include <vector>
#include "umps/messageFormats/message.hpp"
namespace UMPS::ProxyServices::Command
{
 class ModuleDetails;
}
namespace UMPS::ProxyServices::Command
{
/// @class AvailableModulesResponse "availableModulesResponse.hpp" "umps/proxyServices/command//availableModulesResponse.hpp"
/// @brief Returns the available modules that have been registered with the
///        remote module monitoring service.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup UMPS_ProxyServices_Command
class AvailableModulesResponse : public UMPS::MessageFormats::IMessage
{
private:
    using ModuleType = std::vector<ModuleDetails>;
public:
    using iterator = typename ModuleType::iterator;
    using const_iterator = typename ModuleType::const_iterator;
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    AvailableModulesResponse();
    /// @brief Copy constructor.
    /// @param[in] message  The text message class from which to initialize
    ///                     this class.
    AvailableModulesResponse(const AvailableModulesResponse &message);
    /// @brief Move constructor.
    /// @param[in,out] message  The text message class from which to
    ///                         initialize this class.  On exit, message's
    ///                         behavior is undefined.
    AvailableModulesResponse(AvailableModulesResponse &&message) noexcept;
    /// @}

    /// @brief Sets the available modules with which to interact.
    /// @param[in] modules  The available modules.
    /// @throws std::invalid_argument if the module name of any module is
    ///         not set.
    void setModules(const std::vector<ModuleDetails> &modules);
    /// @param[in,out] modules  The available modules.
    /// @throws std::invalid_argument if the module name of any module is
    ///         not set.
    void setModules(std::vector<ModuleDetails> &&modules);
    /// @result The available modules.
    [[nodiscard]] std::vector<ModuleDetails> getModules() const;

    /// @brief Sets the identifier specified in the request.
    /// @param[in] identifier  The request identifier.
    void setIdentifier(int64_t identifier) noexcept;
    /// @result The request identifier.
    [[nodiscard]] int64_t getIdentifier() const noexcept;

    /// @name Operators
    /// @{

    /// @brief Copy assignment.
    /// @param[in] message  The text message class to copy to this.
    /// @result A deep copy of the text message.
    AvailableModulesResponse& operator=(const AvailableModulesResponse &message);
    /// @brief Move assignment.
    /// @param[in,out] message  The text message class whose memory will be
    ///                         moved to this.  On exit, messages's behavior is
    ///                         undefined.
    /// @result The memory from message moved to this.
    AvailableModulesResponse& operator=(AvailableModulesResponse &&message) noexcept;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;
    ModuleDetails& at(size_t pos);
    const ModuleDetails& at(size_t pos) const;
    ModuleDetails& operator[](size_t pos);
    const ModuleDetails& operator[](size_t pos) const;
    /// @}

    /// @name Message Abstract Base Class Properties
    /// @{

    /// @brief Converts the class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const final;
    /// @brief Creates the class from a message.
    /// @param[in] message  The message from which to create this class.
    /// @throws std::runtime_error if the message is invalid.
    void fromMessage(const std::string &message) final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0.
    void fromMessage(const char *data, size_t length) final;
    /// @result A message type indicating the message type.
    [[nodiscard]] std::string getMessageType() const noexcept final;
    /// @result The message version.
    [[nodiscard]] std::string getMessageVersion() const noexcept final;
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept final;
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class and releases all memory.
    void clear() noexcept;
    /// @brief Destructor.
    ~AvailableModulesResponse() override;
    /// @}
private:
    class AvailableModulesResponseImpl;
    std::unique_ptr<AvailableModulesResponseImpl> pImpl;
};
}
#endif
