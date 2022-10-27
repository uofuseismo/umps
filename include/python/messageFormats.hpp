#ifndef UMPS_PYTHON_MESSAGE_FORMATS_HPP
#define UMPS_PYTHON_MESSAGE_FORMATS_HPP
#include <memory>
#include <string>
#include <umps/messageFormats/message.hpp>
#include <pybind11/pybind11.h>
namespace UMPS::Python::MessageFormats
{
/// @class IMessage
/// @brief Defines some add-ons that allow Python to interact with UMPS
///        IMessage-dervied messages.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class IMessage
{
public:
    /// @brief Destructor.
    virtual ~IMessage();
    /// @brief Creates this base class from an UMPS message.
    /// @note This does nothing.
    virtual void fromBaseClass(UMPS::MessageFormats::IMessage &message);
    /// @brief Returns a copy of this class.
    /// @result A null pointer.
    [[nodiscard]] virtual std::unique_ptr<IMessage> clone(const std::unique_ptr<UMPS::MessageFormats::IMessage> &message) const;
    /// @brief Returns an instance of this class.
    /// @result A null pointer.
    [[nodiscard]] virtual std::unique_ptr<IMessage> createInstance() const;
    /// @brief Returns a pointer to the base class.
    /// @result A null pointer.
    [[nodiscard]] virtual std::unique_ptr<UMPS::MessageFormats::IMessage> getBaseClass() const noexcept;
};
void initialize(pybind11::module &m);
}
#endif
