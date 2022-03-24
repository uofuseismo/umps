#ifndef UMPS_PROXYSERVICES_INCREMENTER_COUNTER_HPP
#define UMPS_PROXYSERVICES_INCREMENTER_COUNTER_HPP
#include <memory>
#include <map>
#include <string>
#include <set>
namespace UMPS::ProxyServices::Incrementer
{
/// @class Counter counter.hpp "umps/proxyServices/incrementer/counter.hpp"
/// @brief This thread-safe class is responsible for incrementing item values.
///        This class is a simple way to assign unique identifiers to an item
///        of interest - e.g., a pick or event identifier
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

    /// @param[in] fileName        The name of the sqlite3 file.
    /// @param[in] deleteIfExists  If true then the should the sqlite3 file
    ///                            exist then the underlying table will be 
    ///                            dropped.
    /// @note All items must be added with \c addItem().
    /// @throws std::invalid_argument if there are errors encountered while
    ///         opening the sqlite3 file.
    void initialize(const std::string &fileName,
                    bool deleteIfExists = false);
    /// @param[in] fileName  The name of the sqlite3 file.
    /// @param[in] items     A list of items to increment.  Here, the key is the 
    ///                      item's name and the values are the initial value
    ///                      and increment.  If the increment is not-positive
    ///                      then an error will be thrown.
    /// @param[in] deleteIfExists  If true then the should the sqlite3 file
    ///                            exist then the underlying table will be 
    ///                            dropped.
    /// @throws std::invalid_argument if there are errors encountered while
    ///         opening the sqlite3 file.
    /// @note Items can still be added with \c addItem().
    void initialize(const std::string &fileName,
                    const std::map<std::string, std::pair<int64_t, int32_t>> &items,
                    bool deleteIfExists = false);
    /// @result True indicates that the class is initialized.
    [[nodiscard]] bool isInitialized() const noexcept;
     
    /// @param[in] item          The item's name.
    /// @param[in] initialValue  The initial value of the item.
    /// @param[in] increment     The amount to add to the item when updated.
    /// @throws std::invalid_argument if the increment is not positive
    ///         or \c haveItem(item) is true. 
    void addItem(const std::string &item,
                 int64_t initialValue = 0,
                 int32_t increment = 1);
    /// @param[in] item   The item name to check for existence.
    /// @result True indicates the item was set. 
    [[nodiscard]] bool haveItem(const std::string &item) const;
    /// @}

    /// @name Step 3: Typical Usage
    /// @{

    /// @result The items being incremented. 
    std::set<std::string> getItems() const noexcept;

    /// @brief This function is intended to provide the next available
    ///        unique identifier for the item of interest.
    /// @param[in] item  The item to increment.
    /// @result The next available value in the increment.
    /// @throws std::runtime_error if \c isInitialized() is false or, 
    ///         if by some miracle, your application would exceed the
    ///         largest 64 bit nsigned integer.
    /// @throws std::invalid_argument if \c haveItem() is false.
    [[nodiscard]] int64_t getNextValue(const std::string &item);

    /// @result The current value of the counter.
    /// @param[in] item   The item whose current value will be ascertained.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This is likely not a unique identifier.  Sure, the creation of
    ///       this item with this value may have failed and it doesn't exist but
    ///       it would be dangerous to assume that in a distributed environment.
    ///       This is function merely for your information.
    /// @throws std::invalid_argument if \c haveItem() is false.
    [[nodiscard]] int64_t getCurrentValue(const std::string &item) const;

    /// @brief Resets a specific item.
    /// @param[in] item  The name of the item to reset.
    /// @throws std::invalid_argument if \c haveItem() is false or if
    ///         \c isInitialized() is false.
    void reset(const std::string &item);

    /// @brief Resets all the counters to their initial value.
    /// @throws std::runtime_error if \c isInitialized() is false.
    /// @note This function can cause duplicates identifiers.  Use it with care.
    void reset();
    /// @}


    /// @name Destructors
    /// @{

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
