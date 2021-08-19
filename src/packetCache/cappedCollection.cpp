#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "urts/applications/packetCache/cappedCollection.hpp"
#include "urts/applications/packetCache/circularBuffer.hpp"
#include "urts/logging/logger.hpp"
#include "urts/logging/stdout.hpp"

using namespace URTS::Applications::PacketCache;

namespace
{

}

/// Implementation
template<class T>
class CappedCollection<T>::CappedCollectionImpl
{
public:
    std::map<std::string, CircularBuffer<T>> mCircularBufferMap;
    std::shared_ptr<URTS::Logging::ILogger> mLogger;
};

/// C'tor
template<class T>
CappedCollection<T>::CappedCollection() :
    pImpl(std::unique_ptr<CappedCollectionImpl> ())
{
}

/// Destructor
template<class T>
CappedCollection<T>::~CappedCollection() = default;
 
///--------------------------------------------------------------------------///
///                           Template Instantiation                         ///
///--------------------------------------------------------------------------///
template class URTS::Applications::PacketCache::CappedCollection<double>;
template class URTS::Applications::PacketCache::CappedCollection<float>;
