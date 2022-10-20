#ifndef PYUMPS_MESSAGE_FORMATS_MESSAGE_HPP
#define PYUMPS_MESSAGE_FORMATS_MESSAGE_HPP
#include <memory>
#include <string>
#include "umps/messageFormats/message.hpp"
namespace UMPS::MessageFormats
{
class IMessage;
}
namespace PUMPS::MessageFormats
{
class IMessage
{
public:
    virtual ~IMessage() = default;
    virtual void fromBaseClass(UMPS::MessageFormats::IMessage &message){};
    [[nodiscard]] virtual std::unique_ptr<IMessage> clone(const std::unique_ptr<UMPS::MessageFormats::IMessage> &) const{return nullptr;};
    [[nodiscard]] virtual std::unique_ptr<IMessage> createInstance() const{return nullptr;}
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> getBaseClass() const noexcept{return nullptr;}
};
}
#endif
