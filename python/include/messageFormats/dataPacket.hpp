#ifndef PYUMPS_MESSAGEFORMATS_DATAPACKET_HPP
#define PYUMPS_MESSAGEFORMATS_DATAPACKET_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "messageFormats/message.hpp"
namespace UMPS::MessageFormats
{
template<class T> class DataPacket;
}
namespace PUMPS::MessageFormats
{
class DataPacket : public IMessage
{
public:
    DataPacket();
    virtual ~DataPacket();
    DataPacket(const DataPacket &packet);
    DataPacket(DataPacket &&packet) noexcept;
    DataPacket& operator=(const DataPacket &packet);
    DataPacket& operator=(DataPacket &&packet) noexcept;
    void clear() noexcept;
    UMPS::MessageFormats::DataPacket<double> getNativeClass() const noexcept;
    std::unique_ptr<UMPS::MessageFormats::IMessage> getBaseClass() const noexcept override;
 
    [[nodiscard]] std::string getMessageType() const noexcept;

    // SNCL
    void setNetwork(const std::string &network);
    [[nodiscard]] std::string getNetwork() const;
    void setStation(const std::string &station);
    [[nodiscard]] std::string getStation() const;
    void setChannel(const std::string &channel);
    [[nodiscard]] std::string getChannel() const;
    void setLocationCode(const std::string &location);
    [[nodiscard]] std::string getLocationCode() const;

    void setSamplingRate(double df);
    [[nodiscard]] double getSamplingRate() const;

    void setStartTimeInMicroSeconds(int64_t startTime) noexcept;
    [[nodiscard]] uint64_t getStartTimeInMicroSeconds() const noexcept;
    [[nodiscard]] uint64_t getEndTimeInMicroSeconds() const;

    void setData(pybind11::array_t<double, pybind11::array::c_style | pybind11::array::forcecast> &x);
    void setDataFromVector(const std::vector<double> &x);
    pybind11::array_t<double> getData() const;
    std::vector<double> getDataAsVector() const;

    [[nodiscard]] std::string toJSON(int nSpaces =-1) const;
private:
    std::unique_ptr<UMPS::MessageFormats::DataPacket<double>> pImpl; 
};
void initializeDataPacket(pybind11::module &m);
}
#endif
