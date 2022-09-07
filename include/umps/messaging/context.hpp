#ifndef UMPS_MESSAGING_CONTEXT_HPP
#define UMPS_MESSAGING_CONTEXT_HPP
#include <memory>
#include <cstdint>
namespace UMPS::Messaging
{
/// @class Context context.hpp "umps/messaging/context.hpp"
/// @brief This is a wrapper around the ZeroMQ context.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Context
{
public:
    /// @brief Constructor. 
    Context();
    /// @brief Constructor.
    /// @param[in] nInputOutputThreads  The number of input/output threads.
    ///                                 This is typically 1 for each Gb/s
    ///                                 of communication.  However, if this is
    ///                                 for IPC communication this can be 0.
    explicit Context(int nInputOutputThreads);
    /// @brief Destructor.
    ~Context();

    /// @result A pointer to the ZeroMQ context.
    [[nodiscard]] std::uintptr_t getContext() const;
private:
    class ContextImpl;
    std::unique_ptr<ContextImpl> pImpl; 
};
}
#endif
