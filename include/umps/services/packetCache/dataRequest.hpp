#ifndef UMPS_SERVICES_PACKETCACHE_DATAREQUEST_HPP
#define UMPS_SERVICES_PACKETCACHE_DATAREQUEST_HPP
#include <memory>
#include "umps/messageFormats/message.hpp"
namespace UMPS::Services::PacketCache
{
/// @name DataRequst "dataRequest.hpp" "umps/services/packetCache/dataRequest.hpp"
/// @brief This is a request message for querying the packetCache.  A query
///        consists of a network, station, channel, location code, and 
///        optionally a start time and end time.
/// @note Since the underlying messaging is asynchronous it is to your advantage
///       to provide your request a unique identifier since the requests are
///       not required to filled in the order that they are put on the wire.
/// @sa DataResponse
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class DataRequest : public MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    DataRequest();
    /// @brief Copy constructor.
    /// @param[in] request  The request from which to initialize this class.
    DataRequest(const DataRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] request  The request from which to initialize this class.
    ///                         On exit, requests's behavior is undefined.
    DataRequest(DataRequest &&request) noexcept;
    /// @}
    
    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] request  The request to copy to this.
    /// @result A deep copy of the input request.
    DataRequest& operator=(const DataRequest &request); 
    /// @brief Move assignment operator.
    /// @param[in,out] request  The request whose memory will be moved to this.
    ///                         On exit, request's behavior is undefined.
    /// @result The memory from request moved to this.
    DataRequest& operator=(DataRequest &&request) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief Sets the network code on which the pick was made.
    /// @param[in] network  The network code.
    /// @throws std::invalid_argument if network is empty.
    void setNetwork(const std::string &network);
    /// @result The network code.
    /// @throws std::runtime_error if \c haveNetwork() is false.
    [[nodiscard]] std::string getNetwork() const;
    /// @result True indicates that the network was set.
    [[nodiscard]] bool haveNetwork() const noexcept;

    /// @brief Sets the station name on which the pick was made.
    /// @param[in] station   The station name.
    /// @throws std::invalid_argument if station is empty.
    void setStation(const std::string &station);
    /// @result The station name.
    /// @throws std::runtime_error if \c haveStation() is false.
    [[nodiscard]] std::string getStation() const;
    /// @result True indicates that the station name was set.
    [[nodiscard]] bool haveStation() const noexcept;

    /// @brief Sets the channel name on which the pick was made.
    /// @param[in] channel  The channel name.
    /// @throws std::invalid_argument if channel is empty.
    void setChannel(const std::string &channel);
    /// @result The channel name.
    /// @throws std::runtime_error if the channel was not set.
    [[nodiscard]] std::string getChannel() const;
    /// @result True indicates that the channel was set.
    [[nodiscard]] bool haveChannel() const noexcept;

    /// @brief Sets the location code.
    /// @param[in] location  The location code.
    /// @throws std::invalid_argument if location is empty.
    void setLocationCode(const std::string &location);
    /// @brief Sets the channel code on which the pick was made.
    /// @throws std::runtime_error if \c haveLocationCode() is false.
    [[nodiscard]] std::string getLocationCode() const;
    /// @result True indicates that the location code was set.
    [[nodiscard]] bool haveLocationCode() const noexcept;
    /// @}
    
    /// @name Optional Parameters
    /// @{

    /// @brief This will request data that begins no earlier than the
    ///        given start time and continues to include the most recent packet.
    /// @param[in] startTime  The earliest time in UTC seconds since the epoch.
    ///                       No returned data will begin before this time.
    /// @note If the data has expired in the circular buffer then the 
    ///       earliest time returned will correspond to the oldest packet
    ///       in the cache. 
    void setQueryTime(double startTime);
    /// @brief This will request data that begins no earlier than the
    ///        the given start time and continues up to include the latest
    ///        desired time.
    /// @param[in] queryTimes  queryTimes.first is the start time in UTC seconds
    ///                        since the epoch of the earliest possible data
    ///                        desired.
    ///                        queryTimes.second is the last time in UTC seconds
    ///                        since the epoch of the lsat possible data
    ///                        desired.  Note, since the query operates on the
    ///                        start time of the packet the result may extend
    ///                        slightly beyond this time. 
    /// @throws std::invalid_argument if queryTimes.first >= queryTimes.second.
    void setQueryTimes(const std::pair<double, double> &queryTimes);
    /// @result The earliest time in the request in UTC seconds since the epoch.
    [[nodiscard]] std::pair<double, double> getQueryTimes() const noexcept;

    /// @brief Sets the message identifier.
    /// @param[in] identifier  The message identifier.
    /// @note DataResponse will return this identifier so you can determine
    ///       which requests have been filled.
    void setIdentifier(uint64_t identifier) noexcept;
    /// @result Trhe message identifier.
    [[nodiscard]] uint64_t getIdentifier() const noexcept;
    /// @}

    /// @name Message Properties
    /// @{
    /// @brief Converts the packet class to a string message.
    /// @result The class expressed as a string message.
    /// @throws std::runtime_error if the required information is not set. 
    /// @note Though the container is a string the message need not be
    ///       human readable.
    [[nodiscard]] virtual std::string toMessage() const override final;
    /// @brief Creates the class from a message.
    /// @param[in] data    The contents of the message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NULL or length is 0. 
    virtual void fromMessage(const char *data, size_t length) override final;
    /// @result A message type indicating this is a pick message.
    [[nodiscard]] virtual std::string getMessageType() const noexcept override final;
    /// @name Clone Functions
    /// @{
    /// @result A copy of this class.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> clone() const override final;
    /// @result An uninitialized instance of this class. 
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> createInstance() const noexcept override final;
    /// @}

    /// @name Debugging Utilities
    /// @{

    /// @brief Creates the class from a JSON data request message.
    /// @throws std::runtime_error if the message is invalid.
    void fromJSON(const std::string &message);
    /// @brief Converts the data request class to a JSON message.
    /// @param[in] nIndent  The number of spaces to indent.
    /// @note -1 disables indentation which is preferred for message
    ///       transmission.
    /// @result A JSON representation of this class.
    [[nodiscard]] std::string toJSON(int nIndent =-1) const;
    /// @brief Converts the pick class to a CBOR message.
    /// @result The class expressed in Compressed Binary Object Representation
    ///         (CBOR) format.
    /// @throws std::runtime_error if the required information is not set. 
    [[nodiscard]] std::string toCBOR() const;
    /// @brief Creates the class from a CBOR message.
    /// @param[in] cbor  The CBOR message.
    void fromCBOR(const std::string &cbor);
    /// @brief Creates the class from a CBOR message.
    /// @param[in] data    The contents of the CBOR message.  This is an
    ///                    array whose dimension is [length] 
    /// @param[in] length  The length of data.
    /// @throws std::runtime_error if the message is invalid.
    /// @throws std::invalid_argument if data is NUL or length is 0.
    void fromCBOR(const uint8_t *data, size_t length);
    /// @}

    /// @name Destructors
    /// @{

    /// @brief Resets the class.
    void clear() noexcept;
    /// @brief Destructor.
    ~DataRequest() override;
    /// @}
private:
    class DataRequestImpl;
    std::unique_ptr<DataRequestImpl> pImpl;
};
}
#endif
