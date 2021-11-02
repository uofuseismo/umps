#ifndef UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHEROPTIONS_HPP
#define UMPS_MESSAGING_PUBLISHERSUBSCRIBER_PUBLISHEROPTIONS_HPP
namespace UMPS::Messaging::PublisherSubscriber
{
/// @brief Options for initializing the publisher.
class PublisherOptions
{
public:
    PublisherOptions();
    PublisherOptions(const PublisherOptions &options);
    PublisherOptions(PublisherOptions &&options) const noexcept;

    /// @param[in] address  An address for the publisher to bind to.
    void addAddress(const std::string &address);
    /// @result The addresses to which to bind. 
    /// @throws std::runtime_error if \c haveAddresses is false.
    [[nodiscard]] std::vector<std::string> getAddresses() const;
    /// @result True indicates that address(es) for the publisher to bind to
    ///         have been set.
    [[nodiscard]] bool haveAddresses() const noexcept;

    /// @name Destructors
    /// @{
    void clear() noexcept;
    ~PublisherOptions();
    /// @}  
private:

};
}
#endif
