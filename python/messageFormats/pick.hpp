#ifndef PYURTS_MESSAGEFORMATS_PICK_HPP
#define PYURTS_MESSAGEFORMATS_PICK_HPP
#include <memory>
#include <pybind11/pybind11.h>
namespace URTS::MessageFormats
{
class Pick;
}

namespace PURTS::MessageFormats
{
class Pick
{
public:
    Pick();
    ~Pick();

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


private:
    std::unique_ptr<URTS::MessageFormats::Pick> pImpl;
};
void initializePick(pybind11::module &m);
}
#endif
