#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLEBROADCASTRESPONSE_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLEBROADCASTRESPONSE_HPP
#include <memory>
#include <vector>
#include "umps/services/connectionInformation/enums.hpp"
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::ConnectionInformation
{
 class Details;
}
namespace UMPS::Services::ConnectionInformation
{
/// @class AvailableBroadcastsResponse "availableBroadcastsResponse.hpp" "umps/services/connectionInformation/availableBroadcastsRepsonse.hpp"
/// @brief This class provides the connection information to for all
///        available broadcasts.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class AvailableBroadcastsResponse : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    AvailableBroadcastsResponse();
    /// @brief Copy constructor.
    /// @param[in] response  Creates this class from the given response.
    AvailableBroadcastsResponse(const AvailableBroadcastsResponse &response);
    /// @brief Move constructor.
    /// @param[in,out] response  Creates this class from the given response.
    ///                          On exit, response's behavior is undefined.
    AvailableBroadcastsResponse(AvailableBroadcastsResponse &&response) noexcept;
    /// @}

    /// @name Operator
    /// @{
    /// 
    /// @brief Copy assignment operator.
    /// @result A deep copy of the given response.
    AvailableBroadcastsResponse& operator=(const AvailableBroadcastsResponse &response);
    /// @brief Move asignment operator.
    /// @result The memory from response moved to this.
    AvailableBroadcastsResponse& operator=(AvailableBroadcastsResponse &&response) noexcept;
    /// @}

    /// @name Properties
    /// @{
    /// @brief Sets the details for a connection to the available broadcast.
    /// @throws std::invalid_argument if the details are not properly
    ///         specified.
    void setDetail(const Details &detail);
    /// @brief Sets the connection details for each available broadcast.
    /// @note Use this when there are multiple available broadcasts.
    /// @throws std::invalid_argument if any of the details are not properly
    ///         specified.
    void setDetails(const std::vector<Details> &details);
    /// @result The connection details for every available broadcast.  
    /// @note If this is empty then there are no available broadcasts.
    /// @sa \c getReturnCode().
    [[nodiscard]] std::vector<Details> getDetails() const noexcept;

    /// @brief Allows the incrementer to set its return code and signal to
    ///        the requester whether or not the increment was successful.
    /// @param[in] code   The return code.
    void setReturnCode(ReturnCode code) noexcept;
    /// @result The return code from the incrementer.
    [[nodiscard]] ReturnCode getReturnCode() const noexcept;
    /// @}

    /// @name Message Properties
    /// @{
    /// @brief Create a copy of this class.
    /// @result A copy of this class.
    [[nodiscard]] std::unique_ptr<IMessage> clone() const override final;
    /// @brief Create a clone of this class.
    [[nodiscard]] std::unique_ptr<IMessage> createInstance() const noexcept override final;
    /// @brief Converts this class to a string representation.
    /// @result The class expressed in string format.
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] std::string toMessage() const override final;
    /// @brief Converts this message from a string representation to data.
    void fromMessage(const char *data, const size_t length) override final;
    /// @result The message type.
    [[nodiscard]] std::string getMessageType() const noexcept override final;
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Destructor.
    virtual ~AvailableBroadcastsResponse(); 
    /// @}
private:
    class ResponseImpl;
    std::unique_ptr<ResponseImpl> pImpl; 

};
}
#endif
