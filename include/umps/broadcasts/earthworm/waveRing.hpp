#ifndef UMPS_BROADCASTS_EARTHWORM_WAVERING_HPP
#define UMPS_BROADCASTS_EARTHWORM_WAVERING_HPP
#include <memory>
namespace UMPS
{
 namespace Logging
 {
  class ILog;
 }
 namespace Broadcasts::Earthworm
 {
  template<typename T> class TraceBuf2;
 }
}
namespace UMPS::Broadcasts::Earthworm
{
/// @name WaveRing "waveRing.hpp" "umps/broadcasts/earthworm/waveRing.hpp"
/// @brief A utility for reading from an Earthworm wave ring.
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class WaveRing
{
public:
    /// @name Constructors
    /// @{
    /// @brief Default constructor.
    WaveRing();
    /// @brief Constructor using the given logger.
    /// @param[in] logger  A pointer to the application's logger.
    explicit WaveRing(std::shared_ptr<UMPS::Logging::ILog> &logger);
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
    /// @result True indicates that UMPS has been compiled with the ability
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

    /// @result The traceBuf2 messages read from the ring.
    [[nodiscard]] std::vector<TraceBuf2<double>>
        getTraceBuf2Messages() const noexcept;
    /// @result The number of traceBuf2 messages.
    [[nodiscard]] int getNumberOfTraceBuf2Messages() const noexcept;
    /// @result A pointer to the array of traceBuf2 messages read from the
    ///         ring.  This has dimension [\c getNumberOfTraceBuf2Messages()].
    /// @note This is not recommended for general use.
    [[nodiscard]] const TraceBuf2<double> *getTraceBuf2MessagesPointer() const noexcept;
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
