#ifndef PYUMPS_MESSAGEFORMATS_MESSAGE_HPP
#define PYUMPS_MESSAGEFORMATS_MESSAGE_HPP
#include <memory>
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
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> getBaseClass() const noexcept = 0;
};
void initializeIMessage(pybind11::module &m);
}
#endif
