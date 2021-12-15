#include <string>
#include "umps/broadcasts/dataPacket/subscriberOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/dataPacket.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"

using namespace UMPS::Broadcasts::DataPacket; 
namespace UAuth = UMPS::Messaging::Authentication;

template<class T>
class SubscriberOptions<T>::SubscriberOptionsImpl
{
public:
    SubscriberOptionsImpl()
    {
        mOptions.setHighWaterMark(8192);
        mOptions.setTimeOut(std::chrono::milliseconds{10});
        UMPS::MessageFormats::Messages messageTypes;
        std::unique_ptr<UMPS::MessageFormats::IMessage> dataPacketMessageType
            = std::make_unique<UMPS::MessageFormats::DataPacket<T>> (); 
        messageTypes.add(dataPacketMessageType);
        mOptions.setMessageTypes(messageTypes);
    }
    UMPS::Messaging::PublisherSubscriber::SubscriberOptions mOptions;
};

/// C'tor
template<class T>
SubscriberOptions<T>::SubscriberOptions() :
    pImpl(std::make_unique<SubscriberOptionsImpl> ())
{
}

/// Copy c'tor
template<class T>
SubscriberOptions<T>::SubscriberOptions(const SubscriberOptions &options)
{
    *this = options;
}

/// Move c'tor
template<class T>
SubscriberOptions<T>::SubscriberOptions(SubscriberOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Destructor
template<class T>
SubscriberOptions<T>::~SubscriberOptions() = default;

/// Reset class
template<class T>
void SubscriberOptions<T>::clear() noexcept
{
    pImpl = std::make_unique<SubscriberOptionsImpl> ();
}

/// Copy assignment
template<class T>
SubscriberOptions<T>&
    SubscriberOptions<T>::operator=(const SubscriberOptions<T> &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<SubscriberOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
template<class T>
SubscriberOptions<T>&
    SubscriberOptions<T>::operator=(SubscriberOptions<T> &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Address
template<class T>
void SubscriberOptions<T>::setAddress(const std::string &address)
{
    pImpl->mOptions.setAddress(address);
}

template<class T>
std::string SubscriberOptions<T>::getAddress() const
{
    return pImpl->mOptions.getAddress();
}

template<class T>
bool SubscriberOptions<T>::haveAddress() const noexcept
{
    return pImpl->mOptions.haveAddress();
}

/// High water mark
template<class T>
void SubscriberOptions<T>::setHighWaterMark(const int hwm)
{
    pImpl->mOptions.setHighWaterMark(hwm);
}

template<class T>
int SubscriberOptions<T>::getHighWaterMark() const noexcept
{
    return pImpl->mOptions.getHighWaterMark();
}

/// ZAP options
template<class T>
void SubscriberOptions<T>::setZAPOptions(
    const UMPS::Messaging::Authentication::ZAPOptions &options)
{
    pImpl->mOptions.setZAPOptions(options);
} 

template<class T>
UAuth::ZAPOptions SubscriberOptions<T>::getZAPOptions() const noexcept
{
    return pImpl->mOptions.getZAPOptions();
}

/// Timeout
template<class T>
void SubscriberOptions<T>::setTimeOut(
    const std::chrono::milliseconds timeOut) noexcept
{
    pImpl->mOptions.setTimeOut(timeOut);
}

template<class T>
std::chrono::milliseconds SubscriberOptions<T>::getTimeOut() const noexcept
{
    return pImpl->mOptions.getTimeOut();
}

/// Gets the options
template<class T>
UMPS::Messaging::PublisherSubscriber::SubscriberOptions
    SubscriberOptions<T>::getSubscriberOptions() const noexcept
{
    return pImpl->mOptions;
}

///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------///
template class UMPS::Broadcasts::DataPacket::SubscriberOptions<double>;
template class UMPS::Broadcasts::DataPacket::SubscriberOptions<float>;
template class UMPS::Broadcasts::DataPacket::SubscriberOptions<int>;
