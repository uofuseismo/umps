#ifndef UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLEBROADCASTREQUESTS_HPP
#define UMPS_SERVICES_CONNECTIONINFORMATION_AVAILABLEBROADCASTREQUESTS_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::ConnectionInformation
{
/// @class AvailableBroadcastsRequest "availableBroadcastsRequest.hpp" "umps/services/connectionInformation/availableBroadcastsRequest.hpp"
/// @brief This class requests the connection information for all avialable
///        broadcasts.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class AvailableBroadcastsRequest : public UMPS::MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    AvailableBroadcastsRequest();
    /// @brief Copy constructor.
    /// @param[in] request  Creates this class from the given request.
    AvailableBroadcastsRequest(const AvailableBroadcastsRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] request  Creates this class from the given request.
    ///                         On exit, request's behavior is undefined.
    AvailableBroadcastsRequest(AvailableBroadcastsRequest &&request) noexcept;
    /// @}

    /// @name Operator
    /// @{
    /// 
    /// @brief Copy assignment operator.
    /// @result A deep copy of the given request.
    AvailableBroadcastsRequest& operator=(const AvailableBroadcastsRequest &request);
    /// @brief Move asignment operator.
    /// @result The memory from request moved to this.
    AvailableBroadcastsRequest& operator=(AvailableBroadcastsRequest &&request) noexcept;
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
    virtual ~AvailableBroadcastsRequest(); 
    /// @}
private:
    class BroadcastRequestImpl;
    std::unique_ptr<BroadcastRequestImpl> pImpl; 
};
}
#endif
