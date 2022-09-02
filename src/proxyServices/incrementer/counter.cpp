#include <iostream>
#include <string>
#include <mutex>
#include <vector>
#include <filesystem>
#ifndef NDEBUG
#include <cassert>
#endif
#include <sqlite3.h>
#include "umps/proxyServices/incrementer/counter.hpp"
#include "private/isEmpty.hpp"

using namespace UMPS::ProxyServices::Incrementer;

/*
namespace
{
std::string convertString(const std::string &s) 
{
    auto temp = s;
    temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
    return temp;
}
}
*/

namespace
{

class Row
{
public:
    Row() = default;
    Row(const std::string &itemIn,
        const int64_t valueIn = 0,
        const int32_t incrementIn = 1,
        const int64_t initialValueIn = 0) :
        item(itemIn),
        value(valueIn),
        increment(incrementIn),
        initialValue(initialValueIn)
    {
    }
    std::string item;
    int64_t value = 0;
    int32_t increment = 1;
    int64_t initialValue = 0;
    [[nodiscard]] std::string toString() const
    {
        std::string result = "(\"" + item + "\","  
                           + std::to_string(value) + ","
                           + std::to_string(increment) + ","
                           + std::to_string(initialValue) + ")";
        return result;
    }
};

Row unpack(sqlite3_stmt *result)
{
    Row row;
    std::string item(reinterpret_cast<const char *>
                    (sqlite3_column_text(result, 0)));
    row.item = item;
    row.value = sqlite3_column_int64(result, 1);
    row.increment = sqlite3_column_int(result, 2);
    row.initialValue = sqlite3_column_int64(result, 3);
    return row;
}

[[nodiscard]] std::pair<int, std::string> createCounterTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS counter(";
    sql = sql + "item TEXT UNIQUE NOT NULL, ";
    sql = sql + "value BIG INT DEFAULT 0 NOT NULL, ";
    sql = sql + "increment INTEGER DEFAULT 1, ";
    sql = sql + "initial_value BIG INT DEFAULT 0);"; 
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    std::string outputMessage;
    if (rc != SQLITE_OK)
    {
        outputMessage = errorMessage;
    }
    return std::pair(rc, outputMessage); 
}

[[nodiscard]] std::pair<int, std::string> dropCounterTable(sqlite3 *db)
{
    std::string sql;
    sql = "DROP TABLE IF EXISTS counter";
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    std::string outputMessage;
    if (rc != SQLITE_OK)
    {
        outputMessage = errorMessage;
    }
    return std::pair(rc, outputMessage);
}

[[nodiscard]] bool haveItem(sqlite3 *db, const std::string &item)
{
    bool exists = false;
    std::string sql{"SELECT EXISTS(SELECT 1 FROM counter WHERE item = ?);"};
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL); 
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare query" << std::endl;
        return false;
    }
    rc = sqlite3_bind_text(result, 1, item.c_str(), item.length(), NULL);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to bind text" << std::endl;
        return false;
    }
    int n = 0;
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        exists = sqlite3_column_int(result, 0);
        n = n + 1;
    }
#ifndef NDEBUG
    assert(n == 1);
#endif
    sqlite3_finalize(result);
    return exists;
}

Row getItem(sqlite3 *db, const std::string &item)
{
    Row row;
    std::string sql{"SELECT item, value, increment, initial_value FROM counter WHERE item = ?;"};
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL); 
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare query" << std::endl;
        return row;
    }
    rc = sqlite3_bind_text(result, 1, item.c_str(), item.length(), NULL);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to bind text" << std::endl;
        return row;
    }
    int n = 0;
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        row = unpack(result);
        n = n + 1;
    }
#ifndef NDEBUG
    assert(n <= 1);
#endif
    sqlite3_finalize(result);
    if (n == 0){throw std::runtime_error(item + " does not exist");}
    return row;
}

void getItems(sqlite3 *db, std::vector<Row> *rows)
{
    rows->reserve(1024);
    std::string sql{"SELECT item, value, increment, initial_value FROM counter;"};
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL); 
    if (rc != SQLITE_OK)
    {   
        throw std::runtime_error("Failed to prepare query in getItems");
    }   
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        rows->push_back(unpack(result));
    }   
    sqlite3_finalize(result);
}

void addItem(sqlite3 *db,
             const Row &row)
{
    std::string values;
    std::string sql{"INSERT INTO counter(item, value, increment, initial_value) VALUES "};
    sql = sql + row.toString();
    sql = sql + " ;";
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to add: " + sql + " to counter table";
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }   
     
}

void addItems(sqlite3 *db,
              const std::vector<Row> &rows)
{
    if (rows.empty()){return;}
    auto n = static_cast<int> (rows.size());
    if (n == 1)
    {
        addItem(db, rows[0]);
        return;
    }
    std::string values;
    std::string sql{"INSERT INTO counter(item, value, increment, initial_value) VALUES "};
    for (int i = 0; i < n; i++)
    {
        sql = sql + rows[i].toString();
        if (i < n - 1){sql = sql + ",";}
    }
    sql = sql + " ;";
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to add: " + sql + " to counter table";
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}

[[nodiscard]] int64_t updateItem(sqlite3 *db, const std::string &item)
{
    Row row;
    try
    {
        row = getItem(db, item);
    }
    catch (const std::exception &e)
    {
        row.item = item;
        row.value = 0;
        row.increment = 1;
        row.initialValue = 0;
        addItem(db, row);
        return row.value;
    }
    int64_t nextValue = row.value + row.increment;
    std::string sql = "UPDATE counter SET value = ? WHERE item = ?;";
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL); 
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to prepare query update query");
    }
    rc = sqlite3_bind_int64(result, 1, nextValue);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to bind integer");
    }
    rc = sqlite3_bind_text(result, 2, item.c_str(), item.length(), NULL);
    if (rc != SQLITE_OK)
    {
        throw std::runtime_error("Failed to bind text");
    }
    rc = sqlite3_step(result);
    if (rc != SQLITE_DONE)
    {
        auto errorMessage = sqlite3_errmsg(db);
        std::string error = "Failed to update " + item + " with: "
                          + std::string{errorMessage};
        throw std::runtime_error(error);
    }
    sqlite3_finalize(result);
    return nextValue;
}

}

class Counter::CounterImpl
{
public:
    /// Destructor
    ~CounterImpl()
    {
        closeCounterTable();
    }
    /// Have item?
    bool haveItem(const std::string &item) const
    {
        std::scoped_lock lock(mMutex);
        return ::haveItem(mCounterTable, item);
    }
    /// Add item
    void addItem(const std::string &item,
                 const int64_t initialValue,
                 const int32_t increment)
    {
        std::scoped_lock lock(mMutex);
        if (::haveItem(mCounterTable, item))
        {
            throw std::invalid_argument(item + " already exists");
        }
        auto value = initialValue;
        Row row(item, value, increment, initialValue);
        ::addItem(mCounterTable, row);
    }
    /// Gets the current item
    [[nodiscard]] int64_t getCurrentItemValue(const std::string &item) const
    {
        std::scoped_lock lock(mMutex);
        if (!::haveItem(mCounterTable, item))
        {
            throw std::invalid_argument(item + " does not exist");
        }
        auto row = getItem(mCounterTable, item);
        return row.value;
    }
    /// Gets the item names
    void getItemNames(std::set<std::string> *result)
    {
        std::vector<Row> rows;
        {
        std::scoped_lock lock(mMutex); 
        ::getItems(mCounterTable, &rows);
        }
        for (const auto &row : rows)
        {
            result->insert(row.item);
        }
    }
    /// Add items
    void addItems(const std::map<std::string, std::pair<int64_t, int32_t>> &items)
    {
        if (items.empty()){return;}
        std::vector<Row> rows;
        rows.reserve(items.size());
        std::scoped_lock lock(mMutex);
        for (const auto &item : items)
        {
            auto name = item.first;
            if (::haveItem(mCounterTable, name))
            {
                throw std::invalid_argument(name + " already exists");
            }
            auto initialValue = item.second.first;
            auto increment = item.second.second;
            auto value = initialValue;
            Row row(name, value, increment, initialValue);
            rows.push_back(row);
        }
        ::addItems(mCounterTable, rows);
    }
    /// Update item
    [[nodiscard]] int64_t updateItem(const std::string &item)
    {
        int64_t result = 0;
        std::scoped_lock lock(mMutex);
        if (!::haveItem(mCounterTable, item))
        {
            throw std::invalid_argument(item + " does not exist");
        }
        result = ::updateItem(mCounterTable, item); 
        return result;
    }
    /// Opens the counter table 
    void openCounterTable(const std::string &fileName,
                          const bool deleteIfExists)
    {
        closeCounterTable();
        std::scoped_lock lock(mMutex);
        std::filesystem::path filePath{fileName};
        bool dropTable = false;
        bool createTable = false;
        if (!std::filesystem::exists(filePath))
        {
            auto parentPath = filePath.parent_path();
            if (!parentPath.empty() &&
                !std::filesystem::exists(parentPath))
            {
                if (!std::filesystem::create_directories(parentPath))
                {
                    throw std::runtime_error("Failed to create path: "
                                           + std::string{parentPath});
                }
            }
            createTable = true;
        }
        else
        {
            if (deleteIfExists)
            {
                dropTable = true;
                createTable = true;
            }
        }
        // Open the table
        auto rc = sqlite3_open_v2(fileName.c_str(), &mCounterTable,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mHaveCounterTable = true;
            if (dropTable)
            {
                auto [rcCreate, message] = dropCounterTable(mCounterTable);
                if (rcCreate != SQLITE_OK)
                {
                    closeCounterTable();
                    throw std::runtime_error("Failed to drop table: "
                                           + message);
                }
            }
            if (createTable)
            {
                auto [rcCreate, message] = createCounterTable(mCounterTable);
                if (rcCreate != SQLITE_OK)
                {
                    closeCounterTable();
                    throw std::runtime_error("Failed to create table: "
                                           + message); 
                }
            }
        }
        else
        {
            mCounterTable = nullptr;
            mHaveCounterTable = false;
            throw std::runtime_error("Failed to open counter table: "
                                   + fileName);
        }
    }
    void closeCounterTable()
    {
        std::scoped_lock lock(mMutex);
        if (mHaveCounterTable && mCounterTable){sqlite3_close(mCounterTable);}
        mCounterTable = nullptr;
        mHaveCounterTable = false;
    }
    mutable std::mutex mMutex;
    sqlite3 *mCounterTable{nullptr};
    bool mHaveCounterTable = false;
};

/// C'tor
Counter::Counter() :
    pImpl(std::make_unique<CounterImpl> ()) 
{
}

/// Destructor
Counter::~Counter() = default;

/// Initialize class
void Counter::initialize(const std::string &fileName,
                         const bool deleteIfExists)
{
    pImpl->openCounterTable(fileName, deleteIfExists);
}

void Counter::initialize(
    const std::string &fileName,
    const std::map<std::string, std::pair<int64_t, int32_t>> &items,
    const bool deleteIfExists)
{
    initialize(fileName, deleteIfExists);
    pImpl->addItems(items);
}

/// Initialized?
bool Counter::isInitialized() const noexcept
{
    return pImpl->mHaveCounterTable;
}

/// Add item
void Counter::addItem(const std::string &item,
                      const int64_t initialValue,
                      const int32_t increment)
{
    if (item.empty()){throw std::invalid_argument("Item name is empty");}
    if (increment <= 0)
    {
        throw std::invalid_argument("Increment must be positive");
    }
    if (!isInitialized())
    {
        throw std::runtime_error("Counter table not initialized");
    }
    pImpl->addItem(item, initialValue, increment); // Checks if item exists
}

/// Item exists?
bool Counter::haveItem(const std::string &item) const
{
    if (item.empty()){throw std::invalid_argument("Item name is empty");}
    return pImpl->haveItem(item);
}

/// Get the items be counted in the database
std::set<std::string> Counter::getItems() const noexcept
{
    std::set<std::string> result;
    if (!isInitialized()){return result;}
    pImpl->getItemNames(&result);
    return result;
}

/// Get the current value
int64_t Counter::getCurrentValue(const std::string &item) const
{
    if (item.empty()){throw std::invalid_argument("Item name is empty");}
    if (!isInitialized()){throw std::runtime_error("Table not initialized");}
    return pImpl->getCurrentItemValue(item);
}


/// Update a value
int64_t Counter::getNextValue(const std::string &item)
{
    if (item.empty()){throw std::invalid_argument("Item name is empty");}
    if (!isInitialized()){throw std::runtime_error("Table not initialized");}
    auto nextValue = pImpl->updateItem(item);
    return nextValue;
}

/// Get next value? 
//int64_t Counter::getCurrentValue()

/*
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
*/

/*
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
*/
