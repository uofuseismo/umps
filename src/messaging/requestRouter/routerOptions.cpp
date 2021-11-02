#include <string>
#include <chrono>
#include "umps/messaging/requestRouter/routerOptions.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Messaging::RequestRouter;

class RouterOptions::RouterOptionsImpl
{
public:
    UMPS::Messaging::Authentication::ZAPOptions mZAPOptions;
    std::string mEndPoint;
    std::chrono::milliseconds mPollTimeOutInMilliSeconds{10};
    int mHighWaterMark = 0;
};

/// C'tor
RouterOptions::RouterOptions() :
    pImpl(std::make_unique<RouterOptionsImpl> ())
{
}

/// Copy c'tor
RouterOptions::RouterOptions(const RouterOptions &options)
{
    *this = options;
}

/// Move c'tor
RouterOptions::RouterOptions(RouterOptions &&options) noexcept
{
    *this = std::move(options);
}

/// Copy assignment
RouterOptions& RouterOptions::operator=(const RouterOptions &options)
{
    if (&options == this){return *this;}
    pImpl = std::make_unique<RouterOptionsImpl> (*options.pImpl);
    return *this;
}

/// Move assignment
RouterOptions& RouterOptions::operator=(RouterOptions &&options) noexcept
{
    if (&options == this){return *this;}
    pImpl = std::move(options.pImpl);
    return *this;
}

/// Reset class
void RouterOptions::clear() noexcept
{
    pImpl = std::make_unique<RouterOptionsImpl> ();
}

/// Destructor
RouterOptions::~RouterOptions() = default;

/// End point to bind to
void RouterOptions::setEndPoint(const std::string &endPoint)
{
    if (isEmpty(endPoint))
    {
        throw std::invalid_argument("End point is empty");
    }
    pImpl->mEndPoint = endPoint;    
}

std::string RouterOptions::getEndPoint() const
{
   if (!haveEndPoint()){throw std::runtime_error("End point not set");}
   return pImpl->mEndPoint;
}

bool RouterOptions::haveEndPoint() const noexcept
{
    return !pImpl->mEndPoint.empty();
}

/// ZAP Options
void RouterOptions::setZAPOptions(
    const UMPS::Messaging::Authentication::ZAPOptions &options)
{
    pImpl->mZAPOptions = options;
}

UMPS::Messaging::Authentication::ZAPOptions 
    RouterOptions::getZAPOptions() const noexcept
{
    return pImpl->mZAPOptions;
}

/// High water mark
void RouterOptions::setHighWaterMark(const int highWaterMark)
{
   if (highWaterMark < 0)
   {
       throw std::invalid_argument("High water mark must be non-negative");
   }
   pImpl->mHighWaterMark = highWaterMark;
}

int RouterOptions::getHighWaterMark() const noexcept
{
   return pImpl->mHighWaterMark;
}

/// Sets the timeout
void RouterOptions::setPollTimeOut(
    const std::chrono::milliseconds &timeOut) noexcept
{
    pImpl->mPollTimeOutInMilliSeconds = timeOut;
}

std::chrono::milliseconds RouterOptions::getPollTimeOut() const noexcept
{
    return pImpl->mPollTimeOutInMilliSeconds;
}
