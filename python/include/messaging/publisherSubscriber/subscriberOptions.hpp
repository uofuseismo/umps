#ifndef PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBEROPTIONS_HPP
#define PYUMPS_MESSAGING_PUBLISHERSUBSCRIBER_SUBSCRIBEROPTIONS_HPP
#include <memory>
#include <string>
#include <vector>
#include <pybind11/pybind11.h>
#include "messageFormats/message.hpp"
namespace UMPS::Messaging::PublisherSubscriber
{
 class SubscriberOptions;
}
namespace PUMPS
{
 namespace MessageFormats
 {
  class IMessage;
  class Messages;
 }
 namespace Authentication
 {
  class ZAPOptions;
 }
}
namespace PUMPS::Messaging::PublisherSubscriber
{
class SubscriberOptions
{
public:
    SubscriberOptions();
    SubscriberOptions(const SubscriberOptions &options);
    SubscriberOptions(SubscriberOptions &&options) noexcept;
    SubscriberOptions& operator=(const SubscriberOptions &options);
    SubscriberOptions& operator=(SubscriberOptions &&options) noexcept;
    UMPS::Messaging::PublisherSubscriber::SubscriberOptions getNativeClass() const noexcept;
    ~SubscriberOptions();

    void setAddress(const std::string &address);
    [[nodiscard]] std::string getAddress() const;
    [[nodiscard]] bool haveAddress() const noexcept;

    void setHighWaterMark(int hwm);
    [[nodiscard]] int getHighWaterMark() const noexcept; 

    void setZAPOptions(const PUMPS::Authentication::ZAPOptions &options);
    [[nodiscard]] PUMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;

    void setMessageTypes(const PUMPS::MessageFormats::Messages &messageTypes);
    [[nodiscard]] PUMPS::MessageFormats::Messages getMessageTypes() const;
    [[nodiscard]] bool haveMessageTypes() const noexcept;
    //[[nodiscard]] std::vector<std::unique_ptr<PUMPS::MessageFormats::IMessage>> getPythonMessageTypes() const;

    //void initialize(const SubscriberOptions &options);
    //void addSubscription(const PUMPS::MessageFormats::IMessage &message);

private:
    std::unique_ptr<UMPS::Messaging::PublisherSubscriber::SubscriberOptions>
        pImpl; 
    //std::vector<std::unique_ptr<PUMPS::MessageFormats::IMessage>> mPythonMessageTypes;
};
}
#endif
