#include <iostream>
#include <mutex>
#include <string>
#include <vector>
#include <filesystem>
#include <set>
//#include <zmq.hpp>
#include <sodium/crypto_pwhash.h>
#include <sqlite3.h>
#include <cassert>
#include "umps/messaging/authentication/sqlite3Authenticator.hpp"
#include "umps/messaging/authentication/user.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"
#include "private/authentication/checkIP.hpp"

/// Magic place where ZMQ will send authentication requests to.
#define ZAP_ENDPOINT  "inproc://zeromq.zap.01"

using namespace UMPS::Messaging::Authentication;

namespace
{
struct User
{
    std::string name;
    std::string email;
    std::string password;
    std::string key;
    int privileges;
    int id; 
};
/*
int callback(void *data, int argc, char **argv, char **azColName)
{
   for (int i = 0; i < argc; i++)
   {
       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}
*/
/// Converts a row from the user table to a user
User rowToUser(sqlite3_stmt *result)
{
    User user;
    user.id = sqlite3_column_int(result, 0);
    user.name
       = reinterpret_cast<const char *> (sqlite3_column_text(result, 1));
    if (sqlite3_column_text(result, 3) != NULL)
    {
        user.email = reinterpret_cast<const char *>
                     (sqlite3_column_text(result, 3));
    }
    if (sqlite3_column_text(result, 4) != NULL)
    {
        user.password = reinterpret_cast<const char *>  
                        (sqlite3_column_text(result, 4));
    }
    if (sqlite3_column_text(result, 5) != NULL)
    {
        user.key = reinterpret_cast<const char *>
                   (sqlite3_column_text(result, 5));
    }
    user.privileges = sqlite3_column_int(result, 6); 
    return user;
}
/// Creates the user table
std::pair<int, std::string> createUsersTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS user(";
    sql = sql + "id INT PRIMARY KEY NOT NULL,";
    sql = sql + "name TEXT UNIQUE NOT NULL,";
    sql = sql + "email TEXT NOT NULL,";   
    sql = sql + "password CHAR("   + std::to_string(crypto_pwhash_STRBYTES) + "),";
    sql = sql + "public_key CHAR(" + std::to_string(crypto_pwhash_STRBYTES) + "),";
    sql = sql + "privileges INT NOT NULL);";
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
/// Creates the blacklist table
std::pair<int, std::string> createBlacklistTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS blacklist (";
    sql = sql + "ip TEXT PRIMARY KEY);";
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
/// Creates the whitelist table
std::pair<int, std::string> createWhitelistTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS whitelist (";
    sql = sql + "ip TEXT PRIMARY KEY);";
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
/// Is it whitelisted?
bool checkWhitelisted(sqlite3 *db, const std::string &ip)
{
    bool matches = false;
    // No whitelist -> can't match
    if (db == nullptr)
    {
        return matches;
    }
    std::string sql = "SELECT ip FROM whitelist;";
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK){return matches;}
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        std::string targetIP = reinterpret_cast<const char *>
                               (sqlite3_column_int(result, 0));
        if (ipMatches(ip, targetIP))
        {
            matches = true;
            break;
        }
    }
    sqlite3_finalize(result);
    return matches;
}
/// Is it blacklisted?
bool checkBlacklisted(sqlite3 *db, const std::string &ip)
{
    bool matches = false;
    // No blacklist -> can't match
    if (db == nullptr) 
    {
        return matches;
    }
    std::string sql = "SELECT ip FROM blacklist;";
    sqlite3_stmt *result = nullptr;
    auto rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
    if (rc != SQLITE_OK){return matches;}
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        std::string targetIP = reinterpret_cast<const char *>
                               (sqlite3_column_int(result, 0));
        if (ipMatches(ip, targetIP))
        {
            matches = true;
            break;
        }
    }
    sqlite3_finalize(result);
    return matches;
}

/// Query users from user table
std::vector<User> queryFromUsersTable(sqlite3 *db, const std::string &userName)
{
    std::vector<User> users;
    std::string data("CALLBACK FUNCTION");
    std::string sql;
    sqlite3_stmt *result = nullptr;
    int rc = SQLITE_OK;
    if (userName.empty())
    {
        sql = "SELECT id, name, email, password, public_key, privileges FROM user;";
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
        if (rc != SQLITE_OK)
        {
            return users;
        }
    }
    else
    {
        sql = "SELECT id, name, email, password, public_key, privileges FROM user WHERE name = ?;";
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
        if (rc != SQLITE_OK)
        {
            return users;
        }
        sqlite3_bind_text(result, 1, userName.c_str(), userName.length(), NULL);
    }  
    // Build up the corresponding user from (matching) rows in the table
    users.reserve(1024);
    while (true)
    {
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
        auto user = rowToUser(result);
        users.push_back(std::move(user));
    }
    sqlite3_finalize(result);
    return users;
}
/// @brief This is utility for storing a password by first.
/// @param[in] password   The plain text password to convert to a hashed
///                       password.
/// @param[in] opslimit   Limits the operations.  This will take about
///                       3 seconds on 
/// @param[in] opslimit   Controls the max amount of computations performed
///                       by libsodium.
/// @param[in] memlimit   Controls the max amount of RAM libsodium will use.
/// @result The corresponding hashed string to store in a database.
/// @note The default algorithm will take about 3.5 seconds an a 2.8 GHz
///       Core i7 CPU and require ~1 Gb of RAM.
std::string pwhashString(
    const std::string &password,
    unsigned long long opslimit = crypto_pwhash_OPSLIMIT_SENSITIVE,
    unsigned long long memlimit = crypto_pwhash_MEMLIMIT_SENSITIVE)
{
    std::string hashedPassword;
    hashedPassword.resize(crypto_pwhash_STRBYTES); 
    auto rc = crypto_pwhash_str(hashedPassword.data(),
                                password.c_str(), password.size(),
                                opslimit, memlimit);
    if (rc != 0)
    {
        auto errmsg = "Failed to hash string.  Likely hit memory limit";
        throw std::runtime_error(errmsg);
    }
    return hashedPassword; 
}
/// @brief Verifies a given password matches a hashed password stored in a
///        database.
/// @param[in] password        The password to check against the given hashed
///                            password.
/// @param[in] hashedPassword  The hashed password that exists in the database.
/// @result True indicates the passwords match.
bool doPasswordsMatch(const std::string &password,
                      const std::string &hashedPassword)
{
    assert(hashedPassword.size() == crypto_pwhash_STRBYTES);
    auto rc = crypto_pwhash_str_verify(hashedPassword.c_str(),
                                       password.c_str(),
                                       password.size());
    if (rc != 0){return false;} // Wrong password
    return true;
}
}

class SQLite3Authenticator::AuthenticatorImpl
{
public:
    AuthenticatorImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    explicit AuthenticatorImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    /// Destructor
    ~AuthenticatorImpl()
    {
        closeUsersTable();
        closeBlacklistTable();
        closeWhitelistTable();
    }
    /// Add to blacklist
    void addToBlacklist(const std::string &address)
    {
        std::scoped_lock lock(mMutex);
        if (!mBlacklist.contains(address))
        {
            if (mWhitelist.contains(address))
            {
                auto errmsg = "Remove " + address
                            + " from whitelist before blacklisting";
                mLogger->error(errmsg);
                throw std::invalid_argument(errmsg);
            }
            mLogger->debug("Adding: " + address + " to blacklist");
            mBlacklist.insert(address);
        }
        else
        {
            mLogger->debug("Address: " + address + " already on blacklist");
        }
    }
    /// Remove from blacklist
    void removeFromBlacklist(const std::string &address) noexcept
    {
        std::scoped_lock lock(mMutex);
        if (ipExists(address, mBlacklist))
        {
            mBlacklist.erase(address);
            mLogger->debug("Removing: " + address + " from blacklist");
        }
    }
    /// Add to whitelist
    void addToWhitelist(const std::string &address)
    {
        std::scoped_lock lock(mMutex);
        if (!ipExists(address, mWhitelist))
        {
            if (ipExists(address, mBlacklist))
            {
                auto errmsg = "Remove " + address
                            + " from blacklist before whitelisting";
                mLogger->error(errmsg);
                throw std::invalid_argument(errmsg);
            }
            mLogger->debug("Adding: " + address + " to whitelist");
            mWhitelist.insert(address);
        }
        else
        {
            mLogger->debug("Address: " + address + " already on whitelist");
        }
        
    }
    /// Remove from whitelist
    void removeFromWhitelist(const std::string &address) noexcept
    {
        std::scoped_lock lock(mMutex);
        if (ipExists(address, mWhitelist))
        {
            mWhitelist.erase(address);
            mLogger->debug("Removing: " + address + " from whitelist");
        }
    }
    /// Blacklisted?
    bool isBlacklisted(const std::string &address) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return checkBlacklisted(mBlacklistTable, address);
    }
    /// Whitelisted?
    bool isWhitelisted(const std::string &address) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return checkWhitelisted(mWhitelistTable, address);
    }
    /// Close user database
    void closeUsersTable()
    {
        std::scoped_lock lock(mMutex);
        if (mHaveUsersTable && mUsersTable){sqlite3_close(mUsersTable);}
        mHaveUsersTable = false;
        mUsersTableFile.clear();
        mUsersTable = nullptr;
    }
    /// Close blacklist database
    void closeBlacklistTable()
    {
        std::scoped_lock lock(mMutex);
        if (mHaveBlacklistTable && mBlacklistTable)
        {
            sqlite3_close(mBlacklistTable);
        }
        mHaveBlacklistTable = false;
        mBlacklistTableFile.clear();
        mBlacklistTable = nullptr;
    }
    /// Close white database
    void closeWhitelistTable()
    {
        std::scoped_lock lock(mMutex);
        if (mHaveWhitelistTable && mWhitelistTable)
        {
            sqlite3_close(mWhitelistTable);
        }
        mHaveWhitelistTable = false;
        mWhitelistTableFile.clear();
        mWhitelistTable = nullptr;
    }
    /// Open user database
    int openUsersTable(const std::string &database, bool create)
    {
        int error = 0;
        closeUsersTable();
        std::scoped_lock lock(mMutex);
        auto rc = sqlite3_open_v2(database.c_str(), &mUsersTable,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mHaveUsersTable = true;
            mUsersTableFile = database;
            if (create)
            {
                auto [rcCreate, message] = createUsersTable(mUsersTable);
                if (rcCreate != SQLITE_OK)
                {
                    auto errorMessage = "Failed to create user table: "
                                      + message;
                    mLogger->error(errorMessage);
                    closeUsersTable();
                    error = 1;
                }
            }
        }
        else
        {
            mLogger->error("Failed to open user table: " + database);
            error = 1;
        }
        return error;
    }
    /// Open blacklist database
    int openBlacklistTable(const std::string &database, bool create)
    {
        int error = 0;
        closeBlacklistTable();
        std::scoped_lock lock(mMutex);
        auto rc = sqlite3_open_v2(database.c_str(), &mBlacklistTable,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mHaveBlacklistTable = true;
            mBlacklistTableFile = database;
            if (create)
            {
                auto [rcCreate, message]
                    = createBlacklistTable(mBlacklistTable);
                if (rcCreate != SQLITE_OK)
                {
                    auto errorMessage = "Failed to create blacklist table: "
                                      + message;
                    mLogger->error(errorMessage);
                    closeBlacklistTable();
                    error = 1;
                }   
            }
        }
        else
        {
            mLogger->error("Failed to open blacklist table: " + database);
            error = 1;
        }
        return error;
    }
    /// Open whitelist database
    int openWhitelistTable(const std::string &database, bool create)
    {
        int error = 0;
        closeWhitelistTable();
        std::scoped_lock lock(mMutex);
        auto rc = sqlite3_open_v2(database.c_str(), &mWhitelistTable,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mHaveWhitelistTable = true;
            mWhitelistTableFile = database;
            if (create)
            {
                auto [rcCreate, message]
                    = createWhitelistTable(mWhitelistTable);
                if (rcCreate != SQLITE_OK)
                {
                    auto errorMessage = "Failed to create whitelist table: "
                                      + message;
                    mLogger->error(errorMessage);
                    closeWhitelistTable();
                    error = 1;
                }   
            }
        }
        else
        {
            mLogger->error("Failed to open Whitelist table: " + database);
            error = 1;
        }
        return error;
    }
    /// Have tables?
    bool haveUsersTable() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mHaveUsersTable;
    }
    bool haveBlacklistTable() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mHaveBlacklistTable;
    }
    bool haveWhitelistTable() const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mHaveWhitelistTable;
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::set<std::string> mBlacklist;
    std::set<std::string> mWhitelist;
    sqlite3 *mUsersTable = nullptr;
    sqlite3 *mWhitelistTable = nullptr;
    sqlite3 *mBlacklistTable = nullptr;
    std::string mUsersTableFile;
    std::string mWhitelistTableFile;
    std::string mBlacklistTableFile;
    //std::set<std::pair<std::string, std::string>> mPasswords;
    //bool mHaveZapSocket = false;
    bool mHaveUsersTable = false;
    bool mHaveBlacklistTable = false;
    bool mHaveWhitelistTable = false;
};

/// C'tors
SQLite3Authenticator::SQLite3Authenticator() :
    pImpl(std::make_unique<AuthenticatorImpl> ())
{
}

SQLite3Authenticator::SQLite3Authenticator(
    std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<AuthenticatorImpl> (logger))
{
}

/*
Authenticator::Authenticator(std::shared_ptr<zmq::context_t> &context) :
    pImpl(std::make_unique<AuthenticatorImpl> (context))
{
}

Authenticator::Authenticator(std::shared_ptr<zmq::context_t> &context,
                             std::shared_ptr<UMPS::Logging::ILog> &logger) :
    pImpl(std::make_unique<AuthenticatorImpl> (context, logger))
{
}
*/

/// Destructor
SQLite3Authenticator::~SQLite3Authenticator() = default;

/// Black list
void SQLite3Authenticator::addToBlacklist(const std::string &address)
{
    pImpl->addToBlacklist(address); 
}

ValidationResult SQLite3Authenticator::isBlacklisted(
    const std::string &address) const noexcept
{
    if (!pImpl->isBlacklisted(address))
    {
        return ValidationResult::ALLOWED;
    }
    else
    {
        return ValidationResult::BLACKLISTED;
    }
} 

void SQLite3Authenticator::removeFromBlacklist(const std::string &address) noexcept
{
    return pImpl->removeFromBlacklist(address);
}

/// White list
void SQLite3Authenticator::addToWhitelist(const std::string &address)
{
    pImpl->addToWhitelist(address);
}

ValidationResult SQLite3Authenticator::isWhitelisted(
    const std::string &address) const noexcept
{
    if (pImpl->isWhitelisted(address))
    {
        return ValidationResult::ALLOWED;
    }
    else
    {
        return ValidationResult::BLACKLISTED;
    }
}

void SQLite3Authenticator::removeFromWhitelist(
    const std::string &address) noexcept
{
    return pImpl->removeFromWhitelist(address);
}

/// Open the user table
void SQLite3Authenticator::openUsersTable(
    const std::string &fileName, const bool createIfDoesNotExist)
{
    bool create = false;
    if (!std::filesystem::exists(fileName))
    {
        if (!createIfDoesNotExist)
        {
            throw std::invalid_argument("Users table: " + fileName
                                      + " does not exist");
        }
        auto parentPath = std::filesystem::path(fileName).parent_path();
        if (!std::filesystem::exists(parentPath))
        {
            if (!std::filesystem::create_directories(parentPath))
            {
                throw std::runtime_error("Failed to create directory: "
                                        + std::string(parentPath));
            }
        }
        create = true;
    }
    auto error = pImpl->openUsersTable(fileName, create);
    if (error != 0){throw std::runtime_error("Failed to open user table");}
}

/// Open the blacklist table
void SQLite3Authenticator::openBlacklistTable(
    const std::string &fileName, const bool createIfDoesNotExist)
{
    bool create = false;
    if (!std::filesystem::exists(fileName))
    {
        if (!createIfDoesNotExist)
        {
            throw std::invalid_argument("Users table: " + fileName
                                      + " does not exist");
        }
        auto parentPath = std::filesystem::path(fileName).parent_path();
        if (!std::filesystem::exists(parentPath))
        {
            if (!std::filesystem::create_directories(parentPath))
            {
                throw std::runtime_error("Failed to create directory: "
                                        + std::string(parentPath));
            }
        }
        create = true;
    }
    auto error = pImpl->openBlacklistTable(fileName, create);
    if (error != 0){throw std::runtime_error("Failed to open blacklist table");}
}

/// Open the whitelist table
void SQLite3Authenticator::openWhitelistTable(
    const std::string &fileName, const bool createIfDoesNotExist)
{
    bool create = false;
    if (!std::filesystem::exists(fileName))
    {
        if (!createIfDoesNotExist)
        {
            throw std::invalid_argument("Users table: " + fileName
                                      + " does not exist");
        }
        auto parentPath = std::filesystem::path(fileName).parent_path();
        if (!std::filesystem::exists(parentPath))
        {
            if (!std::filesystem::create_directories(parentPath))
            {
                throw std::runtime_error("Failed to create directory: "
                                        + std::string(parentPath));
            }
        }
        create = true;
    }
    auto error = pImpl->openWhitelistTable(fileName, create);
    if (error != 0){throw std::runtime_error("Failed to open whitelist table");}
}

/// Have tables?
bool SQLite3Authenticator::haveUsersTable() const noexcept
{
    return pImpl->haveUsersTable();
}
bool SQLite3Authenticator::haveBlacklistTable() const noexcept
{
    return pImpl->haveBlacklistTable();
}
bool SQLite3Authenticator::haveWhitelistTable() const noexcept
{
    return pImpl->haveWhitelistTable();
}   

/// Validate
ValidationResult SQLite3Authenticator::isValid(
    const Certificate::UserNameAndPassword &credentials) const noexcept
{
    if (haveUsersTable())
    {

    }
    return ValidationResult::ALLOWED;
}

ValidationResult SQLite3Authenticator::isValid(
    const Certificate::Keys &keys) const noexcept
{
    if (haveUsersTable())
    {

    }
    return ValidationResult::ALLOWED;
}

/*
void Authenticator::start()
{
    pImpl->mLogger->debug("Starting authenticator ZAP socket...");
    stop();
    try
    {
        pImpl->mZapSocket->set(zmq::sockopt::linger, 1);
    }
    catch (const std::exception &e)
    {
        auto error = "Failed to set socket option.  ZMQ failed with: "
                   + std::string(e.what());
        throw std::runtime_error(error);
    }
    try
    {
        pImpl->mZapSocket->connect(ZAP_ENDPOINT);
    }
    catch (const std::exception &e)
    {
        auto error = "Failed to bind to ZAP.  ZMQ failed with: " 
                   + std::string(e.what());
        throw std::runtime_error(error);
    }
    pImpl->mHaveZapSocket = true;
}
*/

/*
void Authenticator::stop()
{
    if (pImpl->mHaveZapSocket)
    {
        pImpl->mLogger->debug("Stopping authenticator ZAP socket...");
//        pImpl->mZapSocket->close();
        pImpl->mHaveZapSocket = false;
    }
}
*/

/// Get a handle on the zap socket
/*
zmq::socket_t* Authenticator::getZapSocket()
{
    return &*pImpl->mZapSocket;
}
*/
