#ifndef URTS_MODULES_INCREMENTER_RESPONSE_HPP
#define URTS_MODULES_INCREMENTER_RESPONSE_HPP
#include <memory>
#include "urts/messageFormats/message.hpp"
#include "urts/modules/icnrementer/enums.hpp"
namespace URTS::Modules::Incrementer
{
/// @class Response "response.hpp" "urts/modules/incrementer/response.hpp"
/// @brief This is a response to an item increment request.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Response : public URTS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Response();
    /// @brief Copy constructor.
    /// @param[in] response  The response from which to initialize this class.
    Response(const Response &response);
    /// @brief Move constructor.
    /// @param[in,out] response  The response from which to initialize this
    ///                          class.  On exit, response's behavior is
    ///                          undefined.
    Response(Response &&response) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Copy assignment operator.
    /// @param[in] response   The response to copy to this.
    /// @result A deep copy of the the input response.
    Response& operator=(const Response &response);
    /// @brief Move assignment operator.
    /// @param[in,out] resonse  The response whose memory will be moved to this.
    ///                         On exit, response's behavior is undefined.
    /// @result The memory from response moved to this.
    Response& operator=(Response &&response) noexcept;
    /// @}

    /// @brief Sets the increment value.
    /// @param[in] value  The increment value.
    void setValue(uint64_t value) noexcept; 
    /// @throws std::runtime_error if the increment value was not set.
    [[nodiscard]] uint64_t getValue() const;
    /// @result True indicates that the increment value was set.
    [[nodiscard]] bool haveValue() const noexcept;

    /// @brief Sets the request identifier.
    /// @param[in] identifier  The request identifier.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result The request identifier.
    [[nodiscard]] uint64_t getIdentifier() const noexcept;

 
    /// @name Destructors
    /// @{
    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    virtual ~Response();
    /// @}
private:
    class ResponseImpl;
    std::unique_ptr<ResponseImpl> pImpl;
};
}
#endif
