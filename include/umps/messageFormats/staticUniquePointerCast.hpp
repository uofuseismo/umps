#ifndef UMPS_MESSAGE_FORMATS_STATIC_UNIQUE_POINTER_CAST_HPP
#define UMPS_MESSAGE_FORMATS_STATIC_UNIQUE_POINTER_CAST_HPP
namespace UMPS::MessageFormats
{
/// @brief Converts a unique pointer of one type to a unique pointer of
///        another type.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
/// @ingroup Messages_BaseClass
template<typename TO, typename FROM>
[[nodiscard]] [[maybe_unused]]
std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM> &&old)
{
    return std::unique_ptr<TO>{static_cast<TO*> (old.release())};
}
}
#endif
