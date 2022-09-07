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

LocalModuleDetails rowToDetails(sqlite3_stmt *result)
{
    LocalModuleDetails details;
    std::string moduleName(reinterpret_cast<const char *>
                           (sqlite3_column_text(result, 0)));
    std::string ipcFile(reinterpret_cast<const char *>
                        (sqlite3_column_text(result, 1))); 
    auto path = std::filesystem::path{ipcFile}.parent_path(); 
    auto id = sqlite3_column_int(result, 2);
    auto status = sqlite3_column_int(result, 3);
    details.setName(moduleName);
    details.setProcessIdentifier(id);
    details.setIPCDirectory(path);
    details.setApplicationStatus(static_cast<ApplicationStatus> (status));
    return details;
}

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
    if (row.haveName())
    {
        values = values + "'" + row.getName() + "',";
    }
    else
    {
        throw std::invalid_argument("Module name not set");
    }

    if (row.haveName())
    {
        values = values + "'" + row.getIPCFileName() + "', ";
    }
    else
    {
        throw std::invalid_argument("IPC file not set");
    }

    values = values + std::to_string(row.getProcessIdentifier()) + ", ";

    values = values
           + std::to_string(static_cast<int> (row.getApplicationStatus()));

    std::string sql;
    sql = "INSERT INTO local_modules (" + fields
        + ") VALUES (" + values + ");";
    return sql;
}

/// @result A command to update a row
std::string updateLocalModule(const LocalModuleDetails &row)
{
    std::string sql = "UPDATE local_modules SET ";
    if (row.haveName())
    {
        sql = sql + "module = '" + row.getName() + "',";
    }
    else
    {
        throw std::invalid_argument("Module name not set");
    }

    if (row.haveName())
    {
        sql = sql + "ipc_file = '" + row.getIPCFileName() + "', ";
    }
    else
    {
        throw std::invalid_argument("IPC file not set");
    }
    sql = sql + "process_identifier = "
        + std::to_string(row.getProcessIdentifier()) + ", ";
    sql = sql + "status = "
           + std::to_string(static_cast<int> (row.getApplicationStatus()));
    sql = sql + " WHERE module = '" + row.getName() + "';";
    return sql;
}

/// Add module
void addModule(sqlite3 *db, const LocalModuleDetails &module)
{
    auto sql = ::addLocalModule(module);
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to add : " + sql
                          + " to table local_modules\n"
                          + "SQLite3 failed with: " + errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}

/// Update module
void updateModule(sqlite3 *db, const LocalModuleDetails &module)
{
    auto sql = ::updateLocalModule(module);
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to add : " + sql 
                          + " to table local_modules\n"
                          + "SQLite3 failed with: " + errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}

/// Module exists?
bool haveModule(sqlite3 *db, const std::string &moduleName)
{
    std::string sql{
        "SELECT EXISTS(SELECT 1 FROM local_modules WHERE module = ?);"};
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to prepare : " + sql 
                          + " statement.";
        throw std::runtime_error(error);
    }
    rc = sqlite3_bind_text(result, 1, moduleName.c_str(),
                           moduleName.length(), NULL);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(result);
        throw std::runtime_error("Failed to set moduleName");
    }
    auto step = sqlite3_step(result);
    if (step != SQLITE_ROW)
    {
        sqlite3_finalize(result);
        throw std::runtime_error("Failed to get row in ::haveModule");
    }
    auto exists = sqlite3_column_int(result, 0);
    sqlite3_finalize(result); 
    return static_cast<bool> (exists);
}

/// Query modules
std::vector<LocalModuleDetails> queryAllModules(sqlite3 *db)
{
    std::string sql{"SELECT * FROM local_modules;"};
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to prepare : " + sql
                          + " statement.";
        throw std::runtime_error(error);
    }
    std::vector<LocalModuleDetails> allDetails;
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        auto details = ::rowToDetails(result);
        allDetails.push_back(details); 
    }
    sqlite3_finalize(result); 
    return allDetails;
}

/// Delete module
int deleteModule(sqlite3 *db, const std::string &moduleName)
{
    std::string sql{"DELETE FROM local_modules WHERE module = ?;"};
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to prepare : " + sql
                          + " statement.";
        throw std::runtime_error(error);
    }
    rc = sqlite3_bind_text(result, 1, moduleName.c_str(),
                           moduleName.length(), NULL);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(result);
        throw std::runtime_error("Failed to set moduleName");
    }
    char *errorMessage = nullptr;
    rc = sqlite3_step(result);
    sqlite3_finalize(result);
    if (rc != SQLITE_DONE)
    {
        sqlite3_free(errorMessage); 
        throw std::runtime_error("Failed to delete module: " + moduleName);
    }
    return 0;
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
        mTableFile = defaultFileName();
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
            mReadOnly = false;
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
            closeTable();
            error = 1;
        }
        return error;
    }
    /// Open the database in read-only mode
    int openReadOnly(const std::filesystem::path &database)
    {
        if (!std::filesystem::exists(database))
        {
            throw std::invalid_argument("Table: " + database.string()
                                      + " does not exist");
        }
        int error = 0;
        closeTable();
        std::scoped_lock lock(mMutex);
        auto rc = sqlite3_open_v2(database.string().c_str(), &mTableHandle,
                                  SQLITE_OPEN_READONLY |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mReadOnly = true;
            mHaveTable = true;
            mTableFile = database;
        }
        else
        {
            closeTable();
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
    /// True indicates the table is read-only
    [[nodiscard]] bool isReadOnly() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mReadOnly;
    }
    /// Module exists?
    bool haveModule(const std::string &details)
    {
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        std::scoped_lock lock(mMutex);
        return ::haveModule(mTableHandle, details);
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
        if (mReadOnly){throw std::runtime_error("Database is readonly");}
        std::scoped_lock lock(mMutex);
        ::addModule(mTableHandle, details);
    }
    /// Update a module
    void updateModule(const LocalModuleDetails &details)
    {
        if (!details.haveName())
        {
            throw std::invalid_argument("Module name not set");
        }
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        if (isReadOnly()){throw std::runtime_error("Database is readonly");}
        if (haveModule(details.getName()))
        {
std::cout << "update" << std::endl;
            std::scoped_lock lock(mMutex);
            ::updateModule(mTableHandle, details);
        }
        else
        {
            addModule(details);
        }
    }
    /// Delete module
    void deleteModule(const std::string &moduleName)
    {
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        if (isReadOnly()){throw std::runtime_error("Database is readonly");}
        std::scoped_lock lock(mMutex);
        ::deleteModule(mTableHandle, moduleName);
    }
    /// Update a module
    std::vector<LocalModuleDetails> queryAllModules() const
    {
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        std::scoped_lock lock(mMutex);
        return ::queryAllModules(mTableHandle);
    }
    static std::filesystem::path defaultFileName()
    {
        return std::filesystem::path{ std::string(std::getenv("HOME"))
                        + "/.local/share/UMPS/tables/localModuleTable.sqlite3"};
    }
///private:
    mutable std::mutex mMutex;
    mutable sqlite3 *mTableHandle{nullptr};
    std::filesystem::path mTableFile = defaultFileName();
    bool mHaveTable{false};
    bool mReadOnly{false};
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

void LocalModuleTable::open(const bool createIfDoesNotExist)
{
    auto fileName = pImpl->mTableFile.string();
    open(fileName, createIfDoesNotExist);
}

void LocalModuleTable::openReadOnly(const std::string &fileName)
{
    if (!std::filesystem::exists(fileName))
    {
        throw std::invalid_argument("Database: " + fileName
                                  + " does not exist");
    }
    auto error = pImpl->openReadOnly(fileName);
    if (error != 0)
    {
        throw std::runtime_error("Failed to open database");
    }
}

void LocalModuleTable::openReadOnly()
{
    auto fileName = pImpl->mTableFile.string();
    openReadOnly(fileName);
}

/// Open?
bool LocalModuleTable::isOpen() const noexcept
{
    return pImpl->haveTable();
}

/// Module exists?
bool LocalModuleTable::haveModule(const LocalModuleDetails &details) const
{
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    return haveModule(details.getName());
}
 
bool LocalModuleTable::haveModule(const std::string &moduleName) const
{
    if (!isOpen()){return false;}
    return pImpl->haveModule(moduleName); 
}

/// Add module
void LocalModuleTable::addModule(const LocalModuleDetails &details)
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    if (haveModule(details))
    {
        throw std::runtime_error("Module: " + details.getName() + " exists");
    }
    pImpl->addModule(details);
}

/// Update module
void LocalModuleTable::updateModule(const LocalModuleDetails &details)
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    pImpl->updateModule(details);
}

/// Delete module
void LocalModuleTable::deleteModule(const LocalModuleDetails &details)
{
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    deleteModule(details.getName());
}

void LocalModuleTable::deleteModule(const std::string &moduleName)
{
    if (!haveModule(moduleName))
    {
        throw std::invalid_argument("Module does not exist");
    }
    pImpl->deleteModule(moduleName);
}

/// Query all modules
std::vector<LocalModuleDetails> LocalModuleTable::queryAllModules() const
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    return pImpl->queryAllModules();
}

/// Closes the table
void LocalModuleTable::close() noexcept
{
    pImpl->closeTable();
}

/// Read-only?
bool LocalModuleTable::isReadOnly() const
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    return pImpl->isReadOnly();
}
