#ifndef PYURTS_MESSAGEFORMATS_MESSAGE_HPP
#define PYURTS_MESSAGEFORMATS_MESSAGE_HPP
#include <memory>
#include "urts/messageFormats/message.hpp"
namespace PURTS::MessageFormats
{
class IMessage
{
public:
    virtual ~IMessage() = default;
    [[nodiscard]] virtual std::unique_ptr<URTS::MessageFormats::IMessage> getBaseClass() const noexcept = 0;
};
}
#endif
