#ifndef UMPS_PYTHON_MESSAGING_HPP
#define UMPS_PYTHON_MESSAGING_HPP  
#include <memory>
#include <pybind11/pybind11.h>
namespace UMPS::Messaging
{
 class Context;
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
void initialize(pybind11::module &m);
}
#endif
