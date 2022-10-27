#ifndef PRIVATE_STATIC_UNIQUEPOINTER_CAST_HPP
#define PRIVATE_STATIC_UNIQUEPOINTER_CAST_HPP
namespace
{
/// @brief Converts a unique pointer of one type to a unique pointer of 
///        another type.
template<typename TO, typename FROM>
[[nodiscard]] [[maybe_unused]]
std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM> &&old)
{
    return std::unique_ptr<TO>{static_cast<TO*> (old.release())};
    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
}
}
#endif
