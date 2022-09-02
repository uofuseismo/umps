#include <iostream>
#include <mutex>
#include <string>
#include <filesystem>
#include <cstdint>
#include <sqlite3.h>
#include "umps/services/command/localModuleTable.hpp"
#include "umps/services/command/localModuleDetails.hpp"

using namespace UMPS::Services::Command;

namespace
{

/*
enum class ProgramStatus : int
{
    NotRunning = 0,
    Running = 1
};

struct Row
{
    std::string mModuleName;
    std::string mIPCFile;
    ProgramStatus mStatus; 
    int64_t mProcessID;
};
*/

/// @result A command to create the sqlite3 table
std::pair<int, std::string> createTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS local_modules(";
    sql = sql + "module TEXT PRIMARY KEY NOT NULL,";
    sql = sql + "ipc_file TEXT UNIQUE NOT NULL,";
    sql = sql + "process_identifier BIGINT NOT NULL,";
    sql = sql + "status INTEGER DEFAULT 0);";
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    std::string outputMessage;
    if (rc != SQLITE_OK)
    {
        outputMessage = errorMessage;
        sqlite3_free(errorMessage);
    }
    return std::pair(rc, outputMessage);
}

/// @result A command to add a row.
std::string addLocalModule(const LocalModuleDetails &row)
{
    std::string fields = "module, ipc_file, process_identifier, status";
    std::string values;
    bool lComma = false;
    if (row.haveName())
    {
        values = values + "'" + row.getName() + "'";
        lComma = true;
    }
    else
    {
        throw std::invalid_argument("Module name not set");
    }

    if (row.haveName())
    {
        if (lComma){values = values + + "', ";}
        values = values + "'" + row.getIPCFileName() + "', ";
        lComma = true;
    }
    else
    {
        throw std::invalid_argument("IPC file not set");
    }

    if (lComma){values = values + + "', ";}
    values = values + std::to_string(row.getProcessIdentifier()) + "', ";

    values = values
           + std::to_string(static_cast<int> (row.getApplicationStatus()));

    std::string sql;
    sql = "INSERT INTO local_processes (" + fields
        + ") VALUES (" + values + ");";
    return sql;
}

/*
/// @result A command to update a row.
std::string updateApplicationRow(const Row &row)
{
     
}

/// @result A command to delete a row.
*/

/// Add module
void addModule(sqlite3 *db, const LocalModuleDetails &module)
{
    auto sql = ::addLocalModule(module);
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to add : " + sql
                          + " to table local_processes\n"
                          + "SQLite3 failed with: " + errorMessage;
        throw std::runtime_error(error);
    }
}


}

class LocalModuleTable::LocalModuleTableImpl
{
public:
    ~LocalModuleTableImpl()
    {
        closeTable();
    }
    /// Close process database
    void closeTable()
    {
        std::scoped_lock lock(mMutex);
        if (mHaveTable && mTableHandle){sqlite3_close(mTableHandle);}
        mHaveTable = false;
        mTableFile.clear();
        mTableHandle = nullptr;
    }
    /// Open database
    int open(const std::filesystem::path &database, bool create)
    {
        int error = 0;
        closeTable();
        std::scoped_lock lock(mMutex);
        auto rc = sqlite3_open_v2(database.string().c_str(), &mTableHandle,
                                  SQLITE_OPEN_READWRITE |
                                  SQLITE_OPEN_CREATE    |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mHaveTable = true;
            mTableFile = database;
            if (create)
            {
                auto [rcCreate, message] = ::createTable(mTableHandle);
                if (rcCreate != SQLITE_OK)
                {
                    auto errorMessage = "Failed to create user table: "
                                      + message;
                    //mLogger->error(errorMessage);
                    closeTable();
                    error = 1;
                }
            }
        }
        else
        {
            //mLogger->error("Failed to open user table: " + database);
            error = 1;
        }
        return error;
    }
    /// True indicates the table is open and ready for use 
    [[nodiscard]] bool haveTable() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mHaveTable;
    }
    /// Add a module to the database
    void addModule(const LocalModuleDetails &details)
    {
        if (!details.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        std::scoped_lock lock(mMutex);
        ::addModule(mTableHandle, details);
    }

///private:
    mutable std::mutex mMutex;
    sqlite3 *mTableHandle{nullptr};
    std::filesystem::path mTableFile 
        = std::filesystem::path{ std::string(std::getenv("HOME"))
                        + "/.local/share/UMPS/tables/localModuleTable.sqlite3"};
    bool mHaveTable{false};
};

/// C'tor
LocalModuleTable::LocalModuleTable() :
    pImpl(std::make_unique<LocalModuleTableImpl> ())
{
}

/// Destructor
LocalModuleTable::~LocalModuleTable() = default;

/// Open file
void LocalModuleTable::open(const std::string &fileName,
                            const bool createIfDoesNotExist)
{
    if (!createIfDoesNotExist)
    {
        if (!std::filesystem::exists(fileName))
        {
            throw std::invalid_argument("Database: " + fileName
                                      + " does not exist");
        }
    }
    auto error = pImpl->open(fileName, createIfDoesNotExist);
    if (error != 0)
    {
        throw std::runtime_error("Failed to open database");
    }
}
