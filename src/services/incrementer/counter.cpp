#include <string>
#include <mutex>
#include "umps/services/incrementer/counter.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::Services::Incrementer;

class Counter::CounterImpl
{
public:
    CounterImpl() = default;
    /// Update the class
    std::pair<int, uint64_t> update()
    {
        std::scoped_lock lock(mMutex);
        if (mCount > mMax){return std::pair<int, uint64_t> (-1, mCount);}
        mCount = mCount + mIncrement;
        return std::pair<int, uint64_t> (0, mCount);
    }
    /// Resets the class
    void clear() noexcept
    {
        std::scoped_lock lock(mMutex);
        mItem.clear();
        mCount = 0;
        mInitialCount = 0;
        mIncrement = 1;
        mMax = std::numeric_limits<uint64_t>::max() - mIncrement;
        mInitialized = false;
    }
    uint64_t currentCount() noexcept
    {
        std::scoped_lock lock(mMutex);
        return mCount;
    }
    /// Reset the counter
    void reset() noexcept
    {
        std::scoped_lock lock(mMutex);
        mCount = mInitialCount;
    }
    /// Destructor
    ~CounterImpl()
    {
        clear();
    }
    bool initialized() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mInitialized;
    }
    void initialize(const std::string &name,
                    const uint64_t count,
                    const uint64_t increment)
    {
        std::scoped_lock lock(mMutex);
        mItem = name;
        mCount = count;
        mInitialCount = count;
        mIncrement = increment;
        mMax = std::numeric_limits<uint64_t>::max() - mIncrement;
        mInitialized = true;
    }
    /// Move c'tor
    CounterImpl(CounterImpl &&counter) noexcept
    {   
        std::scoped_lock lock(counter.mMutex);
        mItem = std::move(counter.mItem);
        mCount = counter.mCount;
        mInitialCount = counter.mInitialCount;
        mIncrement = counter.mIncrement;
        mMax = counter.mMax;
        mInitialized = counter.mInitialized;
    }

    mutable std::mutex mMutex;
    std::string mItem;
    uint64_t mCount = 0;
    uint64_t mInitialCount = 0;
    uint64_t mIncrement = 1;
    uint64_t mMax = std::numeric_limits<uint64_t>::max() - mIncrement;
    bool mInitialized = false;
};

/// C'tor
Counter::Counter() :
    pImpl(std::make_unique<CounterImpl> ())
{
}

/// Move c'tor
Counter::Counter(Counter &&counter) noexcept
{
    *this = std::move(counter);
}

/// Destructor
Counter::~Counter() = default;

/// Clear
void Counter::clear() noexcept
{
    pImpl->clear();
}

/// Reset
void Counter::reset()
{
    pImpl->reset();
}

/// Move assignment
Counter& Counter::operator=(Counter &&counter) noexcept
{
    if (&counter == this){return *this;}
    pImpl = std::make_unique<CounterImpl> (std::move(*counter.pImpl));
    return *this;
}

/// Initialize
void Counter::initialize(const std::string &name,
                         const uint64_t initialValue,
                         const uint64_t increment)
{
    if (increment < 1)
    {
        throw std::invalid_argument("Increment must be positive");
    }
    if (isEmpty(name)){throw std::invalid_argument("Name is empty");}
    pImpl->initialize(name, initialValue, increment);
}

/// Initialized?
bool Counter::isInitialized() const noexcept
{
    return pImpl->initialized();
}

/// Increment
uint64_t Counter::getIncrement() const
{
    if (!isInitialized()){throw std::runtime_error("Counter not initialized");}
    return pImpl->mIncrement;
}

/// Initial value
uint64_t Counter::getInitialValue() const
{
    if (!isInitialized()){throw std::runtime_error("Counter not initialized");}
    return pImpl->mInitialCount;
}

/// Name
std::string Counter::getName() const
{
    if (!isInitialized()){throw std::runtime_error("Counter not initialized");}
    return pImpl->mItem;
}

/// Current value
uint64_t Counter::getCurrentValue() const
{
    if (!isInitialized()){throw std::runtime_error("Counter not initialized");}
    return pImpl->currentCount();
}

/// Next value
uint64_t Counter::getNextValue()
{
    if (!isInitialized()){throw std::runtime_error("Counter not initialized");}
    auto [error, count] = pImpl->update();
    if (error != 0)
    {
        throw std::runtime_error(
            "Overflow detected in counter - recommend reset");
    }
    return count;
}

