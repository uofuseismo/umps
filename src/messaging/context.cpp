#include <cstdint>
#include <zmq.hpp>
#include "umps/messaging/context.hpp"

using namespace UMPS::Messaging;

class Context::ContextImpl
{
public:
    ContextImpl() :
        mContext(zmq::context_t ())
    {
        mContextPtr = &mContext;
    }
    explicit ContextImpl(const int nInputOutputThreads) :
        mContext(zmq::context_t (nInputOutputThreads))
    {
        mContextPtr = &mContext;
    }
    ~ContextImpl()
    {
        mContextPtr = nullptr;
    }
    zmq::context_t mContext;
    void *mContextPtr{nullptr};
};

/// C'tor
Context::Context() :
    pImpl(std::make_unique<ContextImpl> ())
{
}

/// C'tor
Context::Context(const int nInputOutputThreads) :
    pImpl(std::make_unique<ContextImpl> (nInputOutputThreads)) 
{
}

/// Destructor
Context::~Context() = default;

/// Get an opaque integer to the context
uintptr_t Context::getContext() const
{
    return reinterpret_cast<std::uintptr_t> (pImpl->mContextPtr);
}
