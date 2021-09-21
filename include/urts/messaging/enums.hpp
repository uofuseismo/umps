#ifndef URTS_MESSAGING_ENUMS_HPP
#define URTS_MESSAGING_ENUMS_HPP

/// @brief Defines whether or not to use TCP or interprocess communication.
enum class Protocol
{
    TCP = 0,   /*!< Use the TCP protocol. */
    INPROC     /*!< Use interprocess communication. */
};

/// @brief Defines whether or not the messaging pattern will use the CURVE
///        protocol for secure authentication and confidentiality.
enum class Curve
{
    DISABLED = 0, /*!< This indicates the class will not use the
                       curve protocol. */
    ENABLED, /*!< True indicates this will use the CURVE protocol. */
};

#endif
