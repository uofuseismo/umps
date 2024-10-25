#ifndef UMPS_PYTHON_MESSAGING_HPP
#define UMPS_PYTHON_MESSAGING_HPP  
#include <memory>
#include <chrono>
#include <pybind11/pybind11.h>
#include <pybind11/chrono.h>
namespace UMPS
{
 namespace Messaging
 {
  class Context;
  namespace PublisherSubscriber
  {
   class Publisher;
   class PublisherOptions;
   class Subscriber;
   class SubscriberOptions;
  }
 }
 namespace Python
 {
  namespace Authentication
  {
   class ZAPOptions;
  }
  namespace Logging
  {
   class ILog;
  }
  namespace MessageFormats
  {
   class IMessage;
   class Messages;
  }
 }
}
namespace UMPS::Python::Messaging
{
/// @class Context
/// @brief A wrapper for a context.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Context
{
public:
    /// @brief Constructor.
    explicit Context(int nInputThreads = 1);
    /// @brief Destructor.
    ~Context();
    /// @result A shared pointer to the underlying context.
    [[nodiscard]] std::shared_ptr<UMPS::Messaging::Context> getSharedPointer() const;
    Context& operator=(const Context &) = delete;
    Context& operator=(Context &&) noexcept = delete;
private:
    std::shared_ptr<UMPS::Messaging::Context> mContext{nullptr};
};

///--------------------------------------------------------------------------///
///                                 Pub/Sub                                  ///
///--------------------------------------------------------------------------///
namespace PublisherSubscriber
{
/// @class PublisherOptions
/// @brief Wrapper of the publisher options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class PublisherOptions
{
public:
    PublisherOptions();
    PublisherOptions(const PublisherOptions &options);
    explicit PublisherOptions(const UMPS::Messaging::PublisherSubscriber::PublisherOptions &options);
    PublisherOptions(PublisherOptions &&options) noexcept;
    PublisherOptions& operator=(const PublisherOptions &options);
    PublisherOptions& operator=(const  UMPS::Messaging::PublisherSubscriber::PublisherOptions &options);
    PublisherOptions& operator=(PublisherOptions &&options) noexcept; 
    [[nodiscard]] const UMPS::Messaging::PublisherSubscriber::PublisherOptions& getNativeClassReference() const noexcept;
    void setAddress(const std::string &address);
    [[nodiscard]] std::string getAddress() const;
    void setZAPOptions(const UMPS::Python::Authentication::ZAPOptions &options);
    [[nodiscard]] UMPS::Python::Authentication::ZAPOptions getZAPOptions() const noexcept;
    void setSendTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    [[nodiscard]] std::chrono::milliseconds getSendTimeOut() const noexcept;
    void setSendHighWaterMark(int hwm);
    [[nodiscard]] int getSendHighWaterMark() const noexcept;
    void clear() noexcept;
    ~PublisherOptions();
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::PublisherOptions> pImpl; 
};
/// @class SubscriberOptions
/// @brief Wrapper of the subscriber options.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class SubscriberOptions
{
public:
    SubscriberOptions();
    SubscriberOptions(const SubscriberOptions &options);
    explicit SubscriberOptions(const UMPS::Messaging::PublisherSubscriber::SubscriberOptions &options);
    SubscriberOptions(SubscriberOptions &&options) noexcept;
    SubscriberOptions& operator=(const SubscriberOptions &options);
    SubscriberOptions& operator=(const UMPS::Messaging::PublisherSubscriber::SubscriberOptions &options);
    SubscriberOptions& operator=(SubscriberOptions &&options) noexcept;
    [[nodiscard]] const UMPS::Messaging::PublisherSubscriber::SubscriberOptions& getNativeClassReference() const noexcept;
    void setAddress(const std::string &address);
    [[nodiscard]] std::string getAddress() const;
    void setMessageTypes(const UMPS::Python::MessageFormats::Messages &messageTypes);
    [[nodiscard]] UMPS::Python::MessageFormats::Messages getMessageTypes() const;
    void setReceiveHighWaterMark(int hwm);
    [[nodiscard]] int getReceiveHighWaterMark() const noexcept;
    void setReceiveTimeOut(const std::chrono::milliseconds &timeOut) noexcept;
    [[nodiscard]] std::chrono::milliseconds getReceiveTimeOut() const noexcept;
    void setZAPOptions(const UMPS::Python::Authentication::ZAPOptions &options);
    [[nodiscard]] UMPS::Python::Authentication::ZAPOptions getZAPOptions() const noexcept;
    void clear() noexcept;
    ~SubscriberOptions();
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::SubscriberOptions> pImpl;

};
/// @class Publisher
/// @brief Wrapper of the publisher.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Publisher
{
public:
    /// @brief Constructor.
    Publisher();
    /// @brief Constructor with a given context.
    explicit Publisher(UMPS::Python::Messaging::Context &context);
    /// @brief Constructor with a given logger.
    explicit Publisher(UMPS::Python::Logging::ILog &logger);
    /// @brief Constructor with a given context and logger.
    Publisher(UMPS::Python::Messaging::Context &context,
              UMPS::Python::Logging::ILog &logger);
    /// @brief Initializes the publisher.
    void initialize(const PublisherOptions &options);
    /// @brief Sends a message.
    void send(const UMPS::Python::MessageFormats::IMessage &message) const;
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @brief Destructor.
    ~Publisher();
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::Publisher> pImpl;
};
/// @class Subscriber
/// @brief Wrapper of the subscriber.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Subscriber
{
public:
    /// @brief Constructor.
    Subscriber();
    /// @brief Constructor with a given context.
    explicit Subscriber(UMPS::Python::Messaging::Context &context);
    /// @brief Constructor with a given logger.
    explicit Subscriber(UMPS::Python::Logging::ILog &logger);
    /// @brief Constructor with a given context and logger.
    Subscriber(UMPS::Python::Messaging::Context &context,
               UMPS::Python::Logging::ILog &logger);
    /// @brief Initializes the subscriber.
    void initialize(const SubscriberOptions &options);
    /// @result A message.
    [[nodiscard]] std::unique_ptr<UMPS::Python::MessageFormats::IMessage> receive() const;
    /// @result True indicates the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @brief Destructor.
    ~Subscriber();
private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::Subscriber> pImpl;
};

} // End Pub/sub
void initialize(pybind11::module &m);
}
#endif
