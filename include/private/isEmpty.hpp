#ifndef PRIVATE_ISEMPTY_HPP
#define PRIVATE_ISEMPTY_HPP
#include <string>
#include <algorithm>
namespace
{
/// @result True indicates that the string is empty or full of blanks.
[[maybe_unused]] [[nodiscard]]
bool isEmpty(const std::string &s) 
{
    if (s.empty()){return true;}
    return std::all_of(s.begin(), s.end(), [](const char c)
                       {
                           return std::isspace(c);
                       });
}
}
#endif
