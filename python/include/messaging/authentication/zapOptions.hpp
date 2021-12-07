#ifndef PYUMPS_MESSAGING_AUTHENTICATION_ZAPOPTIONS_HPP
#define PYUMPS_MESSAGING_AUTHENTICATION_ZAPOPTIONS_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "umps/messaging/authentication/enums.hpp"
namespace UMPS::Messaging::Authentication
{
 class ZAPOptions;
}
namespace PUMPS::Messaging::Authentication
{
class UserNameAndPassword;
class Keys;
class ZAPOptions
{
public:

    ZAPOptions();
    ZAPOptions(const ZAPOptions &options);
    ZAPOptions(const UMPS::Messaging::Authentication::ZAPOptions &options);
    ZAPOptions(ZAPOptions &&options) noexcept;
    ZAPOptions& operator=(const ZAPOptions &options);
    ZAPOptions& operator=(ZAPOptions &&options) noexcept;
    ZAPOptions& operator=(const UMPS::Messaging::Authentication::ZAPOptions &options);
    UMPS::Messaging::Authentication::ZAPOptions getNativeClass() const noexcept;
    ~ZAPOptions();

    void setGrasslandsClient() noexcept;
    void setGrasslandsServer() noexcept;

    void setStrawhouseClient() noexcept;
    void setStrawhouseServer() noexcept;

    void setWoodhouseClient(const UserNameAndPassword &credentials);
    void setWoodhouseServer() noexcept;

    void setStonehouseClient(const Keys &serverKeys,
                             const Keys &clientKeys);
    void setStonehouseServer(const Keys &serverKeys);

    void setDomain(const std::string &domain);
    std::string getDomain() const noexcept;

    UMPS::Messaging::Authentication::SecurityLevel getSecurityLevel() const noexcept;

    bool isAuthenticationServer() const noexcept;
private:
    std::unique_ptr<UMPS::Messaging::Authentication::ZAPOptions> pImpl; 
};
}
#endif
