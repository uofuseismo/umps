#ifndef UMPS_PROXYSERVICES_PACKETCACHE_BULKDATAREQUEST_HPP
#define UMPS_PROXYSERVICES_PACKETCACHE_BULKDATAREQUEST_HPP
#include <memory>
#include <vector>
#include "umps/messageFormats/message.hpp"
namespace UMPS::ProxyServices::PacketCache
{
class DataRequest;
}
namespace UMPS::ProxyServices::PacketCache
{
/// @name BulkDataRequst "bulkDataRequest.hpp" "umps/proxyServices/packetCache/bulkDataRequest.hpp"
/// @brief This is a request message for querying the packetCache for multiple
///        SNCLs.
/// @note Since the underlying messaging is asynchronous it is to your advantage
///       to provide your request a unique identifier since the requests are
///       not required to filled in the order that they are put on the wire.
/// @sa BulkDataResponse
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class BulkDataRequest : public MessageFormats::IMessage
{
public:
    /// @name Constructors
    /// @{

    /// @brief Constructor.
    BulkDataRequest();
    /// @brief Copy constructor.
    /// @param[in] request  The request from which to initialize this class.
    BulkDataRequest(const BulkDataRequest &request);
    /// @brief Move constructor.
    /// @param[in,out] request  The request from which to initialize this class.
    ///                         On exit, requests's behavior is undefined.
    BulkDataRequest(BulkDataRequest &&request) noexcept;
    /// @}
    
    /// @name Operators
    /// @{

    /// @brief Copy assignment operator.
    /// @param[in] request  The request to copy to this.
    /// @result A deep copy of the input request.
    BulkDataRequest& operator=(const BulkDataRequest &request); 
    /// @brief Move assignment operator.
    /// @param[in,out] request  The request whose memory will be moved to this.
    ///                         On exit, request's behavior is undefined.
    /// @result The memory from request moved to this.
    BulkDataRequest& operator=(BulkDataRequest &&request) noexcept;
    /// @}

    /// @name Required Parameters
    /// @{

    /// @brief Adds a data request.
    /// @param[in] request  The request to add to this bulk request.
    /// @throws std::invalid_argument if request.haveNetwork(),
    ///         request.haveStation(), request.haveChannel(),
    ///         or request.haveLocationCode() is false.
    /// @throws std::invalid_argument if the request already exists.
    /// @sa \c haveDataRequest()
    void addDataRequest(const DataRequest &request);
    /// @result The number of requests.
    [[nodiscard]] int getNumberOfDataRequests() const noexcept;
    /// @result The data requests.
    [[nodiscard]] std::vector<DataRequest> getDataRequests() const noexcept;
    /// @result A pointer to the data requests.  This is an array whose 
    ///         dimension is [getNumberOfRequests()]
    [[nodiscard]] const DataRequest* getDataRequestsPointer() const noexcept;    
    /// @result True indicates the data request already exists.
    [[nodiscard]] bool haveDataRequest(const DataRequest &request) const noexcept;
    /// @brief Clears all the data requests.
    void clearDataRequests() noexcept;
    /// @}
    
    /// @name Optional Parameters
    /// @{

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
    ~BulkDataRequest() override;
    /// @}
private:
    class BulkDataRequestImpl;
    std::unique_ptr<BulkDataRequestImpl> pImpl;
};
}
#endif
