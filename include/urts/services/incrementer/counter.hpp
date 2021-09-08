#ifndef URTS_SERVICES_INCREMENTER_COUNTER_HPP
#define URTS_SERVICES_INCREMENTER_COUNTER_HPP
#include <memory>
namespace URTS::Services::Incrementer
{
/// @class Counter counter.hpp "urts/services/incrementer/counter.hpp"
/// @brief This thread-safe class is responsible for incrementing the specified
///        item's value.  This class is a simple way to assign unique 
///        identifiers to an item of interest - e.g., a pick or event identifier
/// @copyright Ben Baker (University of Utah) distributed under the MIT license.
class Counter
{
public:
    /// @name Constructors
    /// @{
    /// @brief Constructor.
    Counter();
    /// @brief Move constructor.
    /// @param[in] counter  The counter from which to initialize this class.
    ///                     On exit, counter's behavior is undefined.
    Counter(Counter &&counter) noexcept;
    /// @}

    /// @name Operators
    /// @{
    /// @param[in] counter  The counter whose memory will be moved to this.
    ///                     On exit, counter's behavior is undefined.
    /// @result The memory from the operator moved to this.
    Counter& operator=(Counter &&counter) noexcept;
    /// @}

    /// @name Step 1: Initialization
    /// @{
    /// @brief Initializes the class.
    /// @param[in] item          The item name.
    /// @param[in] initialValue  The item's initial value.
    /// @param[in] increment     The item's increment.
    /// @throws std::invalid_argument if the item name is blank or the increment
    ///         is not positive.
    void initialize(const std::string &item,
                    const uint64_t initialValue = 0,
                    const uint64_t increment = 1);
    /// @result True indicates that the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
    /// @result Gets the increment for the counter.
    /// @throws std::runtime_error if \c isInitialized() is false.
    [[nodiscard]] uint64_t getIncrement() const; 
    /// @result The item name that is being counted.
    /// @throws std::runtime_error if \c isInitialized() is false. 
    [[nodiscard]] std::string getName() const;
    /// @result The counter's initial value.
    [[nodiscard]] uint64_t getInitialValue() const;
    /// @}

    /// @name Step 2: Typical Usage
    /// @{
    /// @brief This function is intended to provide the next available
    ///        unique identifier for the item of interest.
    /// @result The next available value in the increment.
    /// @throws std::runtime_error if \c isInitialized() is false or, 
    ///         if by some miracle, your application would exceed the
    ///         largest 64 bit unsigned integer.
    [[nodiscard]] uint64_t getNextValue();

    /// @result The current value of the counter.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This is likely not a unique identifier.  Sure, the creation of
    ///       this item with this value may have failed and it doesn't exist but
    ///       it would be dangerous to assume that in a distributed environment.
    ///       This is function merely for your information.
    [[nodiscard]] uint64_t getCurrentValue() const;

    /// @brief Resets the counter to the initial value.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This function can cause duplicates identifiers.  Use it with care.
    void reset();
    /// @}

    /// @name Destructors
    /// @{ 
    /// @brief Resets the class to an uninitialized state. 
    /// @note If you intend to re-initialize the class it would be smart
    ///       to get the current value and increment it by some non-zero value
    ///       or simply get the next value.  You would then set this as the
    ///       initial value on re-initialization.
    void clear() noexcept;
    /// @brief Destructor.
    ~Counter();
    /// @}

    /// Delete copying - it would be bad to make it easy to copy a counter 
    /// since it that could quickly lead to duplicate "unique" identifiers
    /// for the item of interest.
    Counter(const Counter &counter) = delete;
    Counter& operator=(const Counter &counter) = delete;
private:
    class CounterImpl;
    std::unique_ptr<CounterImpl> pImpl;    
};
}
#endif
