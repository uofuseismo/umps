#ifndef URTS_MESSAGING_EARTHWORM_WAVERING_HPP
#define URTS_MESSAGING_EARTHWORM_WAVERING_HPP
#include <memory>
namespace URTS::Logging
{
class ILog;
};
namespace URTS::Messaging::Earthworm
{
/// @name WaveRing "waveRing.hpp" "urts/messaging/earthworm/waveRing.hpp"
/// @brief A utility for reading from an earthworm wave ring.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class WaveRing
{
public:
    /// @name Constructors
    /// @{
    /// @brief Default constructor.
    WaveRing();
    /// @brief Constructor using the given logger.
    /// @param[in] logger   The logger for this class to use.
    explicit WaveRing(std::shared_ptr<URTS::Logging::ILog> &logger);
    /// @brief Move constructor.
    /// @param[in,out] waveRing  Initializes the ring reader from this class.
    ///                          On exit, waveRing's behavior is undefined.
    WaveRing(WaveRing &&waveRing) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @brief Move assignment operator.
    /// @param[in,out] waveRing  The waveRing class whose memory will be moved
    ///                          to this.  On exit, waveRing's behavior is
    ///                          undefined.
    /// @result The memory from waveRing moved to this.
    WaveRing& operator=(WaveRing &&waveRing) noexcept;
    /// @} 

    /// @name Connection
    /// @{
    /// @result True indicates that URTS has been compiled with the ability
    ///         to use the earthworm library.
    [[nodiscard]] bool haveEarthworm() const noexcept;
    /// @brief Initializes the ring parameters.
    /// @param[in] ringName          The name of the earthworm ring - e.g.,
    ///                              "WAVE_RING".
    /// @param[in] milliSecondsWait  The number of milliseconds to wait after
    ///                              reading from the earthworm ring.
    /// @throws std::
    void connect(const std::string &ringName,
                 const uint32_t milliSecondsWait = 0);
    /// @result True indicates that this class is connected to an
    ///         earthworm ring.
    [[nodiscard]] bool isConnected() const noexcept;
    /// @result The name of the ring to which this class is attached.
    /// @throws std::runtime_error if \c isConnected() is false.
    [[nodiscard]] std::string getRingName() const;
    /// @}

    /// @name Reading
    /// @{
    /// @brief Flushes the ring.  This is usually a good thing to do on startup.
    /// @throws std::runtime_error if \c isConnected() is false.
    void flush();
    /// @brief Reads the ring.
    /// @throws std::runtime_error if \c isConnected() is false.
    void read();
    /// @}

    /// @name Destructors
    /// @{
    /// @brief Disconnects from the ring.  Additionally, all memory is released.
    void disconnect() noexcept;
    /// @brief Destructor.
    ~WaveRing(); 
    /// @}
 
    WaveRing& operator=(const WaveRing &waveRing) = delete;
    WaveRing(const WaveRing &waveRing) = delete;
private:
    class WaveRingImpl;
    std::unique_ptr<WaveRingImpl> pImpl;
};
}
#endif
