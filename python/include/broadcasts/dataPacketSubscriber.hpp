#ifndef PUMPS_BROADCASTS_DATAPACKETSUBSCRIBER_HPP
#define PUMPS_BROADCASTS_DATAPACKETSUBSCRIBER_HPP
#include <memory>
#include <pybind11/pybind11.h>
namespace UMPS::Broadcasts::DataPacket
{
 template<class T> class Subscriber;
}
namespace PUMPS::MessageFormats
{
 class DataPacket;
}
namespace PUMPS::Broadcasts
{
class DataPacketSubscriber
{
public:
    DataPacketSubscriber();
    ~DataPacketSubscriber();
    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]] PUMPS::MessageFormats::DataPacket receive() const;

    DataPacketSubscriber(const DataPacketSubscriber &subscriber) = delete;
    DataPacketSubscriber& operator=(const DataPacketSubscriber &subscriber) = delete;
    DataPacketSubscriber(DataPacketSubscriber &&subscriber) noexcept = delete;
    DataPacketSubscriber& operator=(DataPacketSubscriber &&subscriber) = delete;
private:
    std::unique_ptr< UMPS::Broadcasts::DataPacket::Subscriber<double> > pImpl;
};
}
#endif
