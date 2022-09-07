#include <iostream>
#include <string>
#include "umps/modules/process.hpp"

using namespace UMPS::Modules;

class IProcess::IProcessImpl
{
public:
    void defaultStopCallback()
    {
    }
///private:
    std::function<void ()> mStopCallback{
        std::bind(&IProcessImpl::defaultStopCallback, this)};
};

/// C'tor
IProcess::IProcess() :
    pImpl(std::make_unique<IProcessImpl> ())
{
}

/// Destructor
IProcess::~IProcess() = default;

/// Starts the process
void IProcess::operator()()
{
    start();
}

/// Name
std::string IProcess::getName() const noexcept
{
    return "IProcess";
}

/// Set the stop callback
void IProcess::setStopCallback(const std::function<void ()> &callback)
{
    pImpl->mStopCallback = callback;
}

/// Use the stop callback
void IProcess::issueStopCommand()
{
    pImpl->mStopCallback();
}
