#ifndef PUMPS_BROADCASTS_DATAPACKETSUBSCRIBEROPTIONS_HPP
#define PUMPS_BROADCASTS_DATAPACKETSUBSCRIBEROPTIONS_HPP
#include <memory>
#include <chrono>
#include <pybind11/pybind11.h>
namespace UMPS::Broadcasts::DataPacket
{
 template<class T> class SubscriberOptions;
}
namespace PUMPS::Authentication
{
 class ZAPOptions;
}
namespace PUMPS::Broadcasts
{
class DataPacketSubscriberOptions
{
public:
    DataPacketSubscriberOptions();
    DataPacketSubscriberOptions(const DataPacketSubscriberOptions &options);
    DataPacketSubscriberOptions(DataPacketSubscriberOptions &&options) noexcept;
    void clear() noexcept;
    ~DataPacketSubscriberOptions();

    void setZAPOptions(const PUMPS::Authentication::ZAPOptions &options);
    PUMPS::Authentication::ZAPOptions getZAPOptions() const noexcept;

    void setAddress(const std::string &address);
    [[nodiscard]] std::string getAddress() const;

    void setHighWaterMark(int hwm); 
    [[nodiscard]] int getHighWaterMark() const noexcept;

    void setTimeOut(const int timeOut) noexcept;
    [[nodiscard]] int getTimeOut() const noexcept;

    [[nodiscard]] UMPS::Broadcasts::DataPacket::SubscriberOptions<double> getNativeClass() const noexcept;

    DataPacketSubscriberOptions& operator=(const DataPacketSubscriberOptions &subscriber);
    DataPacketSubscriberOptions& operator=(DataPacketSubscriberOptions &&subscriber) noexcept; 
private:
    std::unique_ptr< UMPS::Broadcasts::DataPacket::SubscriberOptions<double> > pImpl;
};
}
#endif
