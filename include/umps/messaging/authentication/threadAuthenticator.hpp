#ifndef UMPS_MESSAGING_AUTHENTICATION_THREADAUTHENTICATOR_HPP
#define UMPS_MESSAGING_AUTHENTICATION_THREADAUTHENTICATOR_HPP
#include <memory>
namespace zmq 
{
 class context_t;
}
namespace UMPS::Logging
{
 class ILog;
}
namespace UMPS::Messaging::Authentication
{
class ThreadAuthenticator
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    ThreadAuthenticator();
    /// @brief Constructor with a specified logger.
    /// @param[in] logger   The logging utility.
    explicit ThreadAuthenticator(std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @brief Constructor with a specified context.
    /// @param[in] context  The ZeroMQ context to on which to open a socket.
    explicit ThreadAuthenticator(std::shared_ptr<zmq::context_t> &context);
    /// @brief Initializes with a specified context and logger.
    /// @param[in] context  The ZeroMQ context to on which to open a socket.
    /// @param[in] logger   The logging utility.
    ThreadAuthenticator(std::shared_ptr<zmq::context_t> &context,
                        std::shared_ptr<UMPS::Logging::ILog> &logger);
    /// @}

    ~ThreadAuthenticator();

    ThreadAuthenticator(const ThreadAuthenticator &authenticator) = delete;
    ThreadAuthenticator& operator=(const ThreadAuthenticator &authenticator) = delete;
private:
    class ThreadAuthenticatorImpl;
    std::unique_ptr<ThreadAuthenticatorImpl> pImpl;
};
}
#endif
