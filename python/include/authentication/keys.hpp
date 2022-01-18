#ifndef PYUMPS_AUTHENTICATION_KEYS_HPP
#define PYUMPS_AUTHENTICATION_KEYS_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "umps/authentication/enums.hpp"
namespace UMPS::Authentication::Certificate
{
 class Keys;
}
namespace PUMPS::Authentication
{
class Keys
{
public:
    Keys();
    Keys(const Keys &keys);
    Keys(const UMPS::Authentication::Certificate::Keys &keys);
    Keys(Keys &&keys) noexcept;
    Keys& operator=(const Keys &keys);
    Keys& operator=(Keys &&keys) noexcept;
    Keys& operator=(const UMPS::Authentication::Certificate::Keys &keys);
    UMPS::Authentication::Certificate::Keys getNativeClass() const noexcept;
    ~Keys();
    void loadFromTextFile(const std::string &fileName);
    void clear();
private:
    std::unique_ptr<UMPS::Authentication::Certificate::Keys> pImpl;
};
}
#endif
