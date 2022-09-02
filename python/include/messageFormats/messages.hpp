#ifndef PYUMPS_MESSAGEFORMATS_MESSAGES_HPP
#define PYUMPS_MESSAGEFORMATS_MESSAGES_HPP
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace UMPS::MessageFormats
{
 class Messages;
 class IMessage;
}
namespace PUMPS::MessageFormats
{
 class IMessage;
}
namespace PUMPS::MessageFormats
{
class Messages
{
public:
    Messages();
    Messages(const Messages &messages);
    Messages(const UMPS::MessageFormats::Messages &messages);
    Messages(Messages &&messages) noexcept;
    ~Messages();
    UMPS::MessageFormats::Messages getNativeClass() const noexcept;
    Messages& operator=(const Messages &messages);
    Messages& operator=(const UMPS::MessageFormats::Messages &messages);
    Messages& operator=(Messages &&messages) noexcept;

    void add(const IMessage &message); //std::unique_ptr<IMessage> &message);
    [[nodiscard]] int size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    std::unique_ptr<IMessage> get(const std::string &messageType) const;
    std::vector<std::unique_ptr<IMessage>> getPythonMessageTypes() const;
    //[[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> getBaseClass() const noexcept = 0;
private:
    std::unique_ptr<UMPS::MessageFormats::Messages> pImpl;
    std::vector<std::unique_ptr<IMessage>> mPythonMessageTypes;
};
}
#endif
