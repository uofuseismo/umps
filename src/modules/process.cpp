#include <iostream>
#include <string>
#include "umps/modules/process.hpp"

using namespace UMPS::Modules;

/// Starts the process
void IProcess::operator()()
{
    start();
}

/// Destructor
IProcess::~IProcess() = default;

/// Name
std::string IProcess::getName() const noexcept
{
    return "IProcess";
}
