#ifndef URTS_MODULES_INCREMENTER_ENUMS_HPP
#define URTS_MODULES_INCREMENTER_ENUMS_HPP
#include <memory>
#include "urts/messageFormats/message.hpp"
namespace URTS::Modules::Incrementer
{
/// @brief These are predefined set of items that can be incremented by URTS.
/// @note You can define custom items.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class Item
{
    PHASE_PICK,    /*!< A pick item (this is an unassociated arrival). */ 
    PHASE_ARRIVAL, /*!< An arrival item (this is an associated pick). */
    EVENT,         /*!< An event identifier. */
    ORIGIN         /*!< An event can have multiple origins as the location
                        is refined. */
};
/// @brief These are return codes from the incrementer.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
enum class ReturnCode
{
    SUCCESS = 0,           /*!< No errors were detected; the request was succesful. */
    NO_ITEM = 1,           /*!< The desired item could not be found. */
    INVALID_MESSAGE = 2,   /*!< The message could not be parsed. */
    ALGORITHM_FAILURE = 3  /*!< An internal counting error was detected.
                                The returned value should not be trusted to be unique. */
};
}
#endif
