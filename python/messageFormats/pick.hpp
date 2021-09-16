#ifndef PYURTS_MESSAGEFORMATS_PICK_HPP
#define PYURTS_MESSAGEFORMATS_PICK_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "message.hpp"
namespace URTS::MessageFormats
{
class Pick;
}

namespace PURTS::MessageFormats
{
enum class Polarity : int8_t
{
    UNKNOWN = 0,
    UP = 1,
    DOWN =-1
};
class Pick : public IMessage
{
public:
    Pick();
    virtual ~Pick();
    Pick(const Pick &pick);
    Pick(Pick &&pick) noexcept;
    Pick& operator=(const Pick &pick);
    Pick& operator=(Pick &&pick) noexcept;
    URTS::MessageFormats::Pick getNativeClass() const noexcept;
    void clear() noexcept;

    [[nodiscard]] std::string getMessageType() const noexcept;

    void setNetwork(const std::string &network);
    [[nodiscard]] std::string getNetwork() const;

    void setStation(const std::string &station);
    [[nodiscard]] std::string getStation() const;

    void setChannel(const std::string &channel);
    [[nodiscard]] std::string getChannel() const;

    void setLocationCode(const std::string &location);
    [[nodiscard]] std::string getLocationCode() const;

    void setTime(double time) noexcept;
    [[nodiscard]] double getTime() const;

    void setIdentifier(uint64_t id) noexcept;
    [[nodiscard]] uint64_t getIdentifier() const;

    void setPhaseHint(const std::string &hint) noexcept;
    [[nodiscard]] std::string getPhaseHint() const noexcept;

    void setAlgorithm(const std::string &algorithm) noexcept;
    [[nodiscard]] std::string getAlgorithm() const noexcept;

    void setPolarity(Polarity polarity) noexcept;
    Polarity getPolarity() const noexcept;

    [[nodiscard]] std::string toJSON(int nSpaces =-1) const;
private:
    std::unique_ptr<URTS::MessageFormats::Pick> pImpl;
};
void initializePick(pybind11::module &m);
}
#endif
