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
#include "umps/authentication/sqlite3Authenticator.hpp"
#include "umps/authentication/user.hpp"
#include "umps/authentication/certificate/userNameAndPassword.hpp"
#include "umps/authentication/certificate/keys.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"
#include "private/authentication/checkIP.hpp"

/// Magic place where ZMQ will send authentication requests to.
#define ZAP_ENDPOINT  "inproc://zeromq.zap.01"

using namespace UMPS::Authentication;

namespace
{
std::string toPrivilegeString(const UserPrivileges privileges)
{
    if (privileges == UserPrivileges::ReadOnly)
    {
        return "read-only access";
    }
    else if (privileges == UserPrivileges::ReadWrite)
    {
        return "read-write access";
    }
    else if (privileges == UserPrivileges::Administrator)
    {
        return "administrator access";
    }
    else
    {
        return "unkonwn access";
    }
}

struct UserComparitor
{
    bool operator()(const User &lhs, const User &rhs) const
    {
        return lhs.getName() < rhs.getName();
    }
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
    // Identifier
    auto id = sqlite3_column_int(result, 0);
    user.setIdentifier(id);
    // Name
    std::string name(reinterpret_cast<const char *> (sqlite3_column_text(result, 1)));
    user.setName(name);
    // Email
    if (sqlite3_column_text(result, 2) != NULL)
    {
        std::string email(reinterpret_cast<const char *>
                          (sqlite3_column_text(result, 2)));
        user.setEmail(email);
    }
    // Hashed password
    if (sqlite3_column_text(result, 3) != NULL)
    {
        std::string hashedPassword = (reinterpret_cast<const char *>
                                      (sqlite3_column_text(result, 3)));
        user.setHashedPassword(hashedPassword);
    }
    // Public key 
    if (sqlite3_column_text(result, 4) != NULL)
    {
        std::string publicKey(reinterpret_cast<const char *>
                              (sqlite3_column_text(result, 4)));
        user.setPublicKey(publicKey);
        //std::cout << publicKey << " " << publicKey.size() << std::endl;
    }
    // Privileges
    auto privileges
        = static_cast<UserPrivileges> (sqlite3_column_int(result, 5));
    user.setPrivileges(privileges);
    return user;
}

std::string addUserToRow(const User &user)
{
    std::string fields = "name, email, password, public_key, privileges";
    std::string values;
    if (user.haveName())
    {
        values = values + "'" + user.getName() + "', ";
    }
    else
    {
        throw std::invalid_argument("User name not set"); //values = values + "NULL, ";
    }
    if (user.haveEmail())
    {
        values = values + "'" + user.getEmail() + "', ";
    }
    else
    {
        //values = values + "NULL, ";
        throw std::invalid_argument("User email not set");
    }
    if (user.haveHashedPassword())
    {
        values = values + "'" + user.getHashedPassword() + "', ";
    }
    else
    {
        values = values + "NULL, ";
    }
    if (user.havePublicKey())
    {
        values = values + "'" + user.getPublicKey() + "', ";
    }
    else
    {
        values = values + "NULL, ";
    }
    auto privileges = static_cast<int> (user.getPrivileges());
    values = values + std::to_string(privileges);
    std::string sql;
    sql = "INSERT INTO user (" + fields + ") VALUES (" + values + ");";
    return sql;
}

std::string updateUserToRow(const User &user)
{
    std::string sql = "UPDATE user SET ";
    auto id = user.getIdentifier();
    if (user.haveName())
    {
        sql = sql + "name = '" + user.getName() + "', ";
    }   
    else
    {
        //sql = sql + "name = NULL, ";
        throw std::invalid_argument("User name not set");
    }   
    if (user.haveEmail())
    {
        sql = sql + "email = '" + user.getEmail() + "', ";
    }
    else
    {
        //sql = sql + "email = NULL, ";
        throw std::invalid_argument("User email not set");
    }
    if (user.haveHashedPassword())
    {
        sql = sql + "password = '" + user.getHashedPassword() + "', ";
    }   
    else
    {
        sql = sql + "password = NULL, ";
    }
    if (user.havePublicKey())
    {
        sql = sql + "public_key = '" + user.getPublicKey() + "', ";
    }
    else
    {
        sql = sql + "public_key = NULL, ";
    }
    auto privileges = static_cast<int> (user.getPrivileges());
    sql = sql + "privileges = " + std::to_string(privileges);
    sql = sql + " WHERE id = " + std::to_string(id) + ";";
    //std::cout << sql << std::endl;
    return sql;
}

std::string deleteUserFromRow(const User &user)
{
    std::string sql = "DELETE FROM user ";
    auto id = user.getIdentifier();
    sql = sql + " WHERE id = " + std::to_string(id) + ";";
    return sql;
}

/// Creates the user table
std::pair<int, std::string> createUsersTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS user(";
    sql = sql + "id INTEGER PRIMARY KEY AUTOINCREMENT,";
    sql = sql + "name TEXT UNIQUE NOT NULL,";
    sql = sql + "email TEXT NOT NULL,";   
    sql = sql + "password CHAR("   + std::to_string(crypto_pwhash_STRBYTES) + "),";
    sql = sql + "public_key CHAR(40) UNIQUE,";
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
std::vector<User> queryFromUsersTable(sqlite3 *db, const std::string &userNameOrKey,
                                      bool queryUser = true)
{
    std::vector<User> users;
    std::string data("CALLBACK FUNCTION");
    std::string sql;
    sqlite3_stmt *result = nullptr;
    int rc = SQLITE_OK;
    if (userNameOrKey.empty())
    {
        sql = "SELECT id, name, email, password, public_key, privileges FROM user;";
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Failed to prepare query" << std::endl;
            return users;
        }
        users.reserve(1024);
    }
    else
    {
        if (queryUser)
        {
            sql = "SELECT id, name, email, password, public_key, privileges FROM user WHERE name = ?;";
        }
        else
        {
            sql = "SELECT id, name, email, password, public_key, privileges FROM user WHERE public_key = ?;";
        }
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Failed to prepare query" << std::endl; 
            return users;
        }
        rc = sqlite3_bind_text(result, 1, userNameOrKey.c_str(),
                               userNameOrKey.length(), NULL);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Failed to bind text" << std::endl;
            return users;
        }
        users.reserve(1);
    }
    // Build up the corresponding user from (matching) rows in the table
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
/// Add user
void addUserToDatabase(sqlite3 *db, const User &user)
{
    auto sql = addUserToRow(user);
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {
        std::string error = "Failed to add : " + sql + " to table user\n"
                          + "SQLite3 failed with: " + errorMessage;
        throw std::runtime_error(error);
    }
}
/// Update user
void updateUserToDatabase(sqlite3 *db, const User &user)
{
    auto sql = updateUserToRow(user);
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {   
        std::string error = "Failed to update : " + sql + "to table user\n"
                          + "SQLite3 failed with: " + errorMessage;
        throw std::runtime_error(error);
    }   
}
/// Delete user
void deleteUserFromDatabase(sqlite3 *db, const User &user)
{
    auto sql = deleteUserFromRow(user);
    char *errorMessage = nullptr;
    auto rc = sqlite3_exec(db, sql.c_str(), NULL, 0, &errorMessage);
    if (rc != SQLITE_OK)
    {    
        std::string error = "Failed to delete : " + sql + "to table user\n"
                          + "SQLite3 failed with: " + errorMessage;
        throw std::runtime_error(error);
    }
}

}

///--------------------------------------------------------------------------///
///                                Implementation                            ///
///--------------------------------------------------------------------------///
class SQLite3Authenticator::AuthenticatorImpl
{
public:
    AuthenticatorImpl() = delete;
    /*
    AuthenticatorImpl() :
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    */
    AuthenticatorImpl(std::shared_ptr<UMPS::Logging::ILog> logger,
                      const UserPrivileges privileges) :
        mLogger(logger),
        mPrivileges(privileges)
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
    /// Add user
    void addUser(const User &user)
    {
        std::scoped_lock lock(mMutex);
        if (!mHaveUsersTable){throw std::runtime_error("User table not open");}
        auto userName = user.getName();
        constexpr bool queryUser = true;
        auto users = queryFromUsersTable(mUsersTable, userName, queryUser);
        if (users.empty())
        {
            mLogger->info("Adding user: " + userName);
            addUserToDatabase(mUsersTable, user);
        }
        else
        {
            throw std::invalid_argument("User: " + user.getName() + " exists");
        }
    }
    /// Update user - if doesn't exist then add
    void updateUser(const User &user)
    {
        if (!mHaveUsersTable){throw std::runtime_error("User table not open");}
        auto userName = user.getName();
        constexpr bool queryUser = true;
        auto users = queryFromUsersTable(mUsersTable, userName, queryUser);
        if (users.empty())
        {
            mLogger->info("User does not yet exist.  Adding user: " + userName);
            addUserToDatabase(mUsersTable, user);
        }
        else
        {
            auto userWork = user;
#ifndef NDEBUG
            assert(static_cast<int> (users.size()) == 1);
#endif
            mLogger->info("Updating user: " + userName);
            userWork.setIdentifier(users[0].getIdentifier());
            updateUserToDatabase(mUsersTable, userWork);
        }
    }
    /// Delete user
    void deleteUser(const User &user)
    {
        if (!mHaveUsersTable){throw std::runtime_error("User table not open");}
        auto userName = user.getName();
        constexpr bool queryUser = true;
        auto users = queryFromUsersTable(mUsersTable, userName, queryUser);
        if (!users.empty())
        {
#ifndef NDEBUG
            assert(static_cast<int> (users.size()) == 1);
#endif
            mLogger->info("Deleting user: " + userName);
            deleteUserFromDatabase(mUsersTable, users.at(0));
#ifndef NDEBUG
            users = queryFromUsersTable(mUsersTable, userName, queryUser);
            assert(users.empty());
#endif
        }
        else
        {
            throw std::invalid_argument("User: " + user.getName()
                                      + " does not exist");
        }
    }
    /// Load the users
    std::vector<User> loadAllUsers() 
    {
        std::scoped_lock lock(mMutex);
        if (!mHaveUsersTable){throw std::runtime_error("User table not open");}
        const std::string userName{};
        constexpr bool queryUser = true;
        return queryFromUsersTable(mUsersTable, userName, queryUser);
    }
    /// Does user and password match?
    std::pair<std::string, std::string> 
        isValid(const std::string &userName,
                const std::string &password) const
    {
        std::scoped_lock lock(mMutex);
        if (!mHaveUsersTable)
        {
            return std::pair(serverErrorStatus(), "User table not opened");
        }
        // Query all users named userName
        const bool queryUser = true;
        auto returnedUsers = queryFromUsersTable(mUsersTable, userName,
                                                 queryUser);
        // No user exists
        if (returnedUsers.empty())
        {
            return std::pair(clientErrorStatus(),
                            "User: " + userName + " does not exist");
        }
#ifndef NDEBUG 
        assert(static_cast<int> (returnedUsers.size()) == 1); 
#endif
        if (!returnedUsers[0].haveHashedPassword())
        {
            return std::pair(clientErrorStatus(),
                             "User: " + userName + " does not have password");
        }
        if (returnedUsers[0].getPrivileges() < mPrivileges)
        {
            return std::pair(clientErrorStatus(),
                             "User: " + userName
                           + " has insufficient privileges."
                           + userName + " has " 
                           + toPrivilegeString(returnedUsers[0].getPrivileges())
                           + " but requires "
                           + toPrivilegeString(mPrivileges));
        }
        if (!returnedUsers[0].doesPasswordMatch(password))
        {
            return std::pair(clientErrorStatus(),
                             "Given password: " + password
                           + " does not match user's " + userName
                           + " password");
        }
        mLogger->info("Validated user/password for user " + userName
                    + " who has minimum privileges "
                    + toPrivilegeString(mPrivileges));
        return std::pair(okayStatus(), okayMessage());
    }
    /// Does public key match?
    std::pair<std::string, std::string>
        isValid(const std::string &publicKey) const
    {
        std::scoped_lock lock(mMutex);
        if (!mHaveUsersTable)
        {
            return std::pair(serverErrorStatus(), "User table not opened");
        }
        // Query for this public key
        const bool queryUser = false;
        auto returnedUsers = queryFromUsersTable(mUsersTable, publicKey,
                                                 queryUser);
        // No user exists
        if (returnedUsers.empty())
        {
            return std::pair(clientErrorStatus(), "Public key does not exist");
        }
#ifndef NDEBUG 
        assert(static_cast<int> (returnedUsers.size()) == 1);  
#endif
        if (returnedUsers[0].getPrivileges() < mPrivileges)
        {
            return std::pair(clientErrorStatus(),
                            "User has insufficient privileges.  "
                           + returnedUsers[0].getName() + " has " 
                           + toPrivilegeString(returnedUsers[0].getPrivileges())
                           + " but requires "
                           + toPrivilegeString(mPrivileges));
        }
        mLogger->info("Validated public key for user "
                    + returnedUsers[0].getName()
                    + " who has minimum privileges "
                    + toPrivilegeString(mPrivileges));
        return std::pair(okayStatus(), okayMessage()); 
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Logging::ILog> mLogger{nullptr};
    //std::set<User, UserComparitor> mUsers;
    std::set<std::string> mBlacklist;
    std::set<std::string> mWhitelist;
    sqlite3 *mUsersTable{nullptr};
    sqlite3 *mWhitelistTable{nullptr};
    sqlite3 *mBlacklistTable{nullptr};
    std::string mUsersTableFile;
    std::string mWhitelistTableFile;
    std::string mBlacklistTableFile;
    //std::set<std::pair<std::string, std::string>> mPasswords;
    UserPrivileges mPrivileges{UserPrivileges::ReadOnly};
    //bool mHaveZapSocket = false;
    bool mHaveUsersTable{false};
    bool mHaveBlacklistTable{false};
    bool mHaveWhitelistTable{false};
};

/// C'tors
SQLite3Authenticator::SQLite3Authenticator(const UserPrivileges privileges) :
    pImpl(std::make_unique<AuthenticatorImpl> (nullptr, privileges))
{
}

SQLite3Authenticator::SQLite3Authenticator(
    std::shared_ptr<UMPS::Logging::ILog> &logger,
    const UserPrivileges privileges) :
    pImpl(std::make_unique<AuthenticatorImpl> (logger, privileges))
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

std::pair<std::string, std::string> SQLite3Authenticator::isBlacklisted(
    const std::string &address) const noexcept
{
    if (!pImpl->isBlacklisted(address))
    {
        return std::pair(okayStatus(), "OK");
    }
    else
    {
        return std::pair("400",
                         "Address: " + address + " is blacklisted");
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

std::pair<std::string, std::string> SQLite3Authenticator::isWhitelisted(
    const std::string &address) const noexcept
{
    if (pImpl->isWhitelisted(address))
    {
        pImpl->mLogger->info("Address: " + address + " is whitelisted");
        return std::pair(okayStatus(), "OK");
    }
    else
    {
        pImpl->mLogger->info("Address: " + address + " is blacklisted");
        return std::pair("400",
                         "Address: " + address + " is blacklisted");
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

/// Add user
void SQLite3Authenticator::addUser(const User &user)
{
    // Check input
    if (!user.haveName()){throw std::invalid_argument("User name not set");} 
    if (!user.haveEmail()){throw std::invalid_argument("User email not set");}
    // Attempt to add to database
    pImpl->addUser(user);
}

/// Update user
void SQLite3Authenticator::updateUser(const User &user)
{
    // Check input
    if (!user.haveName()){throw std::invalid_argument("User name not set");} 
    if (!user.haveEmail()){throw std::invalid_argument("User email not set");}
    // Attempt to add to database
    pImpl->updateUser(user);
}

/// Delete a user
void SQLite3Authenticator::deleteUser(const User &user)
{
    // Check input
    if (!user.haveName()){throw std::invalid_argument("User name not set");} 
    if (!user.haveEmail()){throw std::invalid_argument("User email not set");}
    // Attempt to delete user from database
    pImpl->deleteUser(user);
}

/// Get all user info
std::vector<User> SQLite3Authenticator::getUsers() const
{
    return pImpl->loadAllUsers();
}

/// Validate
std::pair<std::string, std::string> SQLite3Authenticator::isValid(
    const Certificate::UserNameAndPassword &credentials) const noexcept
{
    if (!haveUsersTable())
    {
        return std::pair(serverErrorStatus(), "User table not loaded");
    }
    if (!credentials.haveUserName())
    {
        return std::pair(serverErrorStatus(),
                         "User name not set on credentials");
    }
    if (!credentials.havePassword())
    {
        return std::pair(serverErrorStatus(),
                         "Password not set on credentials");
    }
    return pImpl->isValid(credentials.getUserName(),
                          credentials.getPassword());
}

std::pair<std::string, std::string> SQLite3Authenticator::isValid(
    const Certificate::Keys &keys) const noexcept
{
    if (!haveUsersTable())
    {
        return std::pair(serverErrorStatus(), "user table not loaded");
    }
    if (!keys.havePublicKey())
    {
        return std::pair(serverErrorStatus(),
                         "Public key not set on keys");
    }
    auto textKey = std::string(keys.getPublicTextKey().data());
    return pImpl->isValid(textKey);
}

/// Minimum privileges
UserPrivileges SQLite3Authenticator::getMinimumUserPrivileges() const noexcept
{
    return pImpl->mPrivileges;
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
