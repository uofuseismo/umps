#include <iostream>
#include <mutex>
#include <string>
#include <set>
#include <zmq.hpp>
#include <sodium/crypto_pwhash.h>
#include <sqlite3.h>
#include "umps/messaging/authentication/sqlite3Authenticator.hpp"
#include "umps/logging/stdout.hpp"
#include "umps/logging/log.hpp"

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
/// Creates the user table
std::pair<int, std::string> createUserTable(sqlite3 *db)
{
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS user(";
    sql = sql + "id INT PRIMARY KEY NOT NULL,";
    sql = sql + "name TEXT UNIQUE NOT NULL,";
    sql = sql + "email TEXT NOT NULL,";   
    sql = sql + "password CHAR("   + std::to_string(crypto_pwhash_STRBYTES) + "),";
    sql = sql + "public_key CHAR(" + std::to_string(crypto_pwhash_STRBYTES) + "),";
    sql = sql + "privileges INT NOT NULL)";
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
    sql = "CREATE TABLE IF NOT EXISTS blacklist(";
    sql = sql + "ip TEXT PRIMARY KEY)";
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
    sql = "CREATE TABLE IF NOT EXISTS whitelist(";
    sql = sql + "ip TEXT PRIMARY KEY)";
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
/// Query users from user table
std::vector<User> queryFromUserTable(sqlite3 *db, const std::string &userName)
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
        sql = "SELECT id, name, email, password, public_key, privileges FROM user WHERE name = ?";
        rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &result, NULL);
        if (rc != SQLITE_OK)
        {
            return users;
        }
        sqlite3_bind_text(result, 1, userName.c_str(), userName.length(), NULL);
    }  
    // Build up the corresponding user from (matching) rows in the table
    while (true)
    {
        User user;
        auto step = sqlite3_step(result);
        if (step != SQLITE_ROW){break;}
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
/*
        mContext(std::make_shared<zmq::context_t> (1)),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)),
*/
        mLogger(std::make_shared<UMPS::Logging::StdOut> ())
    {
    }
    explicit AuthenticatorImpl(std::shared_ptr<UMPS::Logging::ILog> &logger) :
/*
        mContext(std::make_shared<zmq::context_t> (1)),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)),
*/
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> ();
        }
    }
    ~AuthenticatorImpl()
    {
        close();
    }
/*
    explicit AuthenticatorImpl(std::shared_ptr<zmq::context_t> &context) :
        mContext(context),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)), 
        mLogger(std::make_shared<UMPS::Logging::StdOut> ()) 
    {
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating context");
            mContext = std::make_shared<zmq::context_t> (1);
            mZapSocket = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::rep);
        }
    }
    AuthenticatorImpl(std::shared_ptr<zmq::context_t> &context,
                      std::shared_ptr<UMPS::Logging::ILog> &logger) :
        mContext(context),
        mZapSocket(std::make_unique<zmq::socket_t> (*mContext,
                                                    zmq::socket_type::rep)), 
        mLogger(logger)
    {
        if (logger == nullptr)
        {
            mLogger = std::make_shared<UMPS::Logging::StdOut> (); 
        }
        if (context == nullptr)
        {
            mLogger->warn("Context is NULL - creating cotnext");
            mContext = std::make_shared<zmq::context_t> (1);
            mZapSocket = std::make_unique<zmq::socket_t> (
                *mContext, zmq::socket_type::rep);
        }
    }
*/
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
        if (mBlacklist.contains(address))
        {
            mBlacklist.erase(address);
            mLogger->debug("Removing: " + address + " from blacklist");
        }
    }
    /// Add to whitelist
    void addToWhitelist(const std::string &address)
    {
        std::scoped_lock lock(mMutex);
        if (!mWhitelist.contains(address))
        {
            if (mBlacklist.contains(address))
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
        if (mWhitelist.contains(address))
        {
            mWhitelist.erase(address);
            mLogger->debug("Removing: " + address + " from whitelist");
        }
    }
    /// Blacklisted?
    bool isBlacklisted(const std::string &address) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mBlacklist.contains(address);
    }
    /// Whitelisted?
    bool isWhitelisted(const std::string &address) const noexcept
    {
        std::scoped_lock lock(mMutex);
        return mWhitelist.contains(address);
    }
    /// Close database
    void close()
    {
        std::scoped_lock lock(mMutex);
        if (mHaveUserTable && mUserTable){sqlite3_close(mUserTable);}
        mHaveUserTable = false;
        mUserTable = nullptr;
    }
    /// Open database
    void openUserTable(const std::string &database)
    {
        close();
        std::scoped_lock lock(mMutex);
        auto rc = sqlite3_open_v2(database.c_str(), &mUserTable,
                                  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                  SQLITE_OPEN_FULLMUTEX,
                                  nullptr);
        if (rc == SQLITE_OK)
        {
            mHaveUserTable = true;
        }
        else
        {
            mLogger->error("Failed to open user table: " + database);
            mUserTable = nullptr;
            mHaveUserTable = false;
        }
    }
///private:
    mutable std::mutex mMutex;
    std::shared_ptr<UMPS::Logging::ILog> mLogger;
    std::set<std::string> mBlacklist;
    std::set<std::string> mWhitelist;
    sqlite3 *mUserTable = nullptr;
    sqlite3 *mWhitelistTable = nullptr;
    sqlite3 *mBlacklistTable = nullptr;
    //std::set<std::pair<std::string, std::string>> mPasswords;
    //bool mHaveZapSocket = false;
    bool mHaveUserTable = false;
    bool mHaveBlacklistTable = false;
    bool mHaveWhiteListTable = false;
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

bool SQLite3Authenticator::isBlacklisted(const std::string &address) const noexcept
{
    return pImpl->isBlacklisted(address);
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

bool SQLite3Authenticator::isWhitelisted(
    const std::string &address) const noexcept
{
    return pImpl->isWhitelisted(address);
}

void SQLite3Authenticator::removeFromWhitelist(
    const std::string &address) noexcept
{
    return pImpl->removeFromWhitelist(address);
}

/// Start the authenticator
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
