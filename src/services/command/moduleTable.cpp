#include <iostream>
#include <mutex>
#include <string>
#include <filesystem>
#include <cstdint>
#include <sqlite3.h>
#include "umps/services/command/moduleTable.hpp"
#include "umps/services/command/moduleDetails.hpp"

using namespace UMPS::Services::Command;

namespace
{

ModuleDetails rowToDetails(sqlite3_stmt *result)
{
    ModuleDetails details;
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
    details.setApplicationStatus(
        static_cast<ModuleDetails::ApplicationStatus> (status));
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
std::string addModule(const ModuleDetails &row)
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
std::string updateModule(const ModuleDetails &row)
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
void addModule(sqlite3 *db, const ModuleDetails &module)
{
    auto sql = ::addModule(module);
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
void updateModule(sqlite3 *db, const ModuleDetails &module)
{
    auto sql = ::updateModule(module);
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
std::vector<ModuleDetails> queryAllModules(sqlite3 *db)
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
    std::vector<ModuleDetails> allDetails;
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

/// Query specific module
ModuleDetails queryModule(sqlite3 *db, const std::string &moduleName)
{
    ModuleDetails details;
    std::string sql{"SELECT * FROM local_modules WHERE module = ? LIMIT 1;"};
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
    details = ::rowToDetails(result);
    sqlite3_finalize(result);
    if (rc != SQLITE_ROW)
    {
        sqlite3_free(errorMessage);
        throw std::runtime_error("Failed to query module: " + moduleName);
    }
    return details;
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

class ModuleTable::ModuleTableImpl
{
public:
    ~ModuleTableImpl()
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
    bool haveModule(const std::string &details) const
    {
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        std::scoped_lock lock(mMutex);
        return ::haveModule(mTableHandle, details);
    }
    /// Add a module to the database
    void addModule(const ModuleDetails &details)
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
    void updateModule(const ModuleDetails &details)
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
    /// Query all modules
    std::vector<ModuleDetails> queryAllModules() const
    {
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        std::scoped_lock lock(mMutex);
        return ::queryAllModules(mTableHandle);
    }
    /// Query a specific momdule
    ModuleDetails queryModule(const std::string &moduleName) const
    {
        if (!haveTable())
        {
            throw std::runtime_error("Local module table not open");
        }
        if (!haveModule(moduleName))
        {
            throw std::invalid_argument("Module " + moduleName
                                      + " not in table");
        }
        std::scoped_lock lock(mMutex);
        return ::queryModule(mTableHandle, moduleName);
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
ModuleTable::ModuleTable() :
    pImpl(std::make_unique<ModuleTableImpl> ())
{
}

/// Destructor
ModuleTable::~ModuleTable() = default;

/// Open file
void ModuleTable::open(const std::string &fileName,
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
    else
    {
        // Make sure directory exists
        std::filesystem::path filePath(fileName);
        auto parentPath = filePath.parent_path();
        if (!parentPath.empty())
        {
            if (!std::filesystem::exists(parentPath))
            {
                if (!std::filesystem::create_directories(parentPath))
                {
                     throw std::runtime_error("Failed to create directory: "
                                            + std::string{parentPath});
                }
            }
        }
    }
    auto error = pImpl->open(fileName, createIfDoesNotExist);
    if (error != 0)
    {
        throw std::runtime_error("Failed to open database");
    }
}

void ModuleTable::open(const bool createIfDoesNotExist)
{
    auto fileName = pImpl->mTableFile.string();
    open(fileName, createIfDoesNotExist);
}

void ModuleTable::openReadOnly(const std::string &fileName)
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

void ModuleTable::openReadOnly()
{
    auto fileName = pImpl->mTableFile.string();
    openReadOnly(fileName);
}

/// Open?
bool ModuleTable::isOpen() const noexcept
{
    return pImpl->haveTable();
}

/// Module exists?
/*
bool ModuleTable::haveModule(const ModuleDetails &details) const
{
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    return haveModule(details.getName());
}
*/
/// Module exists? 
bool ModuleTable::haveModule(const std::string &moduleName) const
{
    if (!isOpen()){return false;}
    return pImpl->haveModule(moduleName); 
}

/// Add module
void ModuleTable::addModule(const ModuleDetails &details)
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    if (haveModule(details.getName()))
    {
        throw std::runtime_error("Module: " + details.getName() + " exists");
    }
    pImpl->addModule(details);
}

/// Update module
void ModuleTable::updateModule(const ModuleDetails &details)
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    pImpl->updateModule(details);
}

/// Delete module
void ModuleTable::deleteModule(const ModuleDetails &details)
{
    if (!details.haveName())
    {
        throw std::invalid_argument("Module name not set");
    }
    deleteModule(details.getName());
}

void ModuleTable::deleteModule(const std::string &moduleName)
{
    if (!haveModule(moduleName))
    {
        throw std::invalid_argument("Module does not exist");
    }
    pImpl->deleteModule(moduleName);
}

/// Query all modules
std::vector<ModuleDetails> ModuleTable::queryAllModules() const
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    return pImpl->queryAllModules();
}

/// Query a module
ModuleDetails ModuleTable::queryModule(const std::string &name) const
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    return pImpl->queryModule(name);
}

/// Closes the table
void ModuleTable::close() noexcept
{
    pImpl->closeTable();
}

/// Read-only?
bool ModuleTable::isReadOnly() const
{
    if (!isOpen()){throw std::runtime_error("Table not open");}
    return pImpl->isReadOnly();
}
