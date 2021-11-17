#include <zmq.hpp>
#include <thread>
#include <sodium/crypto_pwhash.h>
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messaging/authentication/zapOptions.hpp"
#include "umps/messaging/authentication/user.hpp"
#include "umps/messaging/authentication/sqlite3Authenticator.hpp"
#include "umps/messaging/authentication/service.hpp"
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messaging/publisherSubscriber/subscriberOptions.hpp"
#include "umps/messageFormats/messages.hpp"
#include "umps/messageFormats/pick.hpp"
#include "umps/logging/stdout.hpp"
#include "private/staticUniquePointerCast.hpp"
#include "private/authentication/checkIP.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace UMPS::Messaging::Authentication;

/*
TEST(Messaging, IAuthenticator)
{
    EXPECT_EQ(IAuthenticator::okayStatus(), std::string("200"));
    EXPECT_EQ(IAuthenticator::okayMessage(), std::string("OK"));
    EXPECT_EQ(IAuthenticator::clientErrorStatus(), std::string("400"));
    EXPECT_EQ(IAuthenticator::serverErrorStatus(), std::string("500"));
}

TEST(Messaging, isOkayIP)
{
    EXPECT_TRUE(isOkayIP("123.345.323.44"));
    EXPECT_FALSE(isOkayIP("*"));
    EXPECT_FALSE(isOkayIP("122.*.*.233"));
    EXPECT_FALSE(isOkayIP("*.121.233.233"));
    EXPECT_TRUE(isOkayIP("123.*"));
    EXPECT_TRUE(isOkayIP("123.34.*"));
    EXPECT_TRUE(isOkayIP("122.35.23.*"));
}

TEST(Messaging, CheckIP)
{
    std::set<std::string> ipAddresses;
    ipAddresses.insert("127.12.24.58");
    EXPECT_TRUE(ipExists("127.12.24.58",  ipAddresses));
    EXPECT_FALSE(ipExists("127.12.24.47", ipAddresses));

    ipAddresses.insert("128.*");
    EXPECT_TRUE(ipExists("128.488.832.12", ipAddresses));   

    EXPECT_TRUE(ipExists("128.488.*", ipAddresses));
    ipAddresses.insert("128.488.*");
    EXPECT_TRUE(ipExists("128.489.832.12", ipAddresses));

    EXPECT_FALSE(ipExists("44.883.85.848", ipAddresses));
}

TEST(Messaging, CertificateUserNameAndPassword)
{
    const std::string userName = "user";
    const std::string password = "password";
    //const std::string password= "o8cZ?nWv7h[jtY]*<H&CT<CL(f{zGz&Z6LK9o[EL";
    Certificate::UserNameAndPassword plainText;
    EXPECT_FALSE(plainText.haveUserName());
    EXPECT_FALSE(plainText.havePassword());
 
    plainText.setUserName(userName);
    EXPECT_FALSE(plainText.havePassword());
    EXPECT_EQ(plainText.getUserName(), userName);
     
    plainText.clear();
    plainText.setPassword(password);
    EXPECT_FALSE(plainText.haveUserName());
    EXPECT_EQ(plainText.getPassword(), password);

    plainText.setUserName(userName);
 
    Certificate::UserNameAndPassword plainTextCopy(plainText);
    EXPECT_EQ(plainTextCopy.getUserName(), userName);
    EXPECT_EQ(plainTextCopy.getPassword(), password);

    std::string passwordInteractive;
    EXPECT_NO_THROW(passwordInteractive =
                    plainTextCopy.getHashedPassword(Certificate::HashLevel::INTERACTIVE));
    //std::cout << passwordInteractive << std::endl;
}

TEST(Messaging, CertificateKeys)
{
    Certificate::Keys certificate;
    const std::string metadata = "Test metadata";
    EXPECT_NO_THROW(certificate.create());
    EXPECT_TRUE(certificate.haveKeyPair());
    certificate.setMetadata(metadata);
    EXPECT_EQ(metadata, certificate.getMetadata());
    auto publicTextKey  = certificate.getPublicTextKey();
    auto privateTextKey = certificate.getPrivateTextKey(); 
    auto publicKey  = certificate.getPublicKey();
    auto privateKey = certificate.getPrivateKey();

    Certificate::Keys fromText;
    fromText.setPublicKey(publicTextKey);
    fromText.setPrivateKey(privateTextKey);
    EXPECT_EQ(fromText.getPublicKey(),  certificate.getPublicKey());
    EXPECT_EQ(fromText.getPrivateKey(), certificate.getPrivateKey());
  
    Certificate::Keys fromBinary;
    fromBinary.setPublicKey(publicKey); 
    fromBinary.setPrivateKey(privateKey);
    EXPECT_EQ(fromBinary.getPublicTextKey(),  certificate.getPublicTextKey());
    EXPECT_EQ(fromBinary.getPrivateTextKey(), certificate.getPrivateTextKey());

    // Do something funky to mess with some down-deep logic
    certificate.setMetadata("");
    certificate.writePublicKeyToTextFile("temp.public_key");
    // Do something funky to mess with some down-deep logic
    certificate.setMetadata(metadata);
    certificate.writePrivateKeyToTextFile("temp.private_key");
 
    Certificate::Keys fromFile;
    fromFile.loadFromTextFile("temp.public_key");
    EXPECT_EQ(fromFile.getMetadata(), "");
    fromFile.loadFromTextFile("temp.private_key");
    EXPECT_EQ(fromFile.getMetadata(), certificate.getMetadata());
    EXPECT_EQ(fromFile.getPublicTextKey(), certificate.getPublicTextKey());
    EXPECT_EQ(fromFile.getPrivateTextKey(), certificate.getPrivateTextKey());
    EXPECT_EQ(fromFile.getPublicKey(), certificate.getPublicKey());
    EXPECT_EQ(fromFile.getPrivateKey(), certificate.getPrivateKey());

    std::remove("temp.public_key");
    std::remove("temp.private_key");
}

TEST(Messaging, User)
{
    User user;
    int id = 12345;
    const std::string name = "user";
    const std::string email = "user@domain.com";
    // Password is password with INTERACTIVE hashing
    const std::string password = "password";
    // Senstive
    //const std::string hashedPassword{"$argon2id$v=19$m=1048576,t=4,p=1$JOU27drJpXOrscvIaX1yUw$Q3PHo5x7hh41aY296u4Yp5E4V+Dc0Qho/ncB3+ocRCI"};
    // Moderate 
    //const std::string hashedPassword{"$argon2id$v=19$m=262144,t=3,p=1$oF32CH2F+surUFHAULqM4Q$kUrRKyxjfzIAF1AtIoz88l5/rcn/9WhO6AyzKPIuDQg"};
    // Interactive
    const std::string hashedPassword{"$argon2id$v=19$m=65536,t=2,p=1$H1VXeJPv1KHeUT7cb6TluA$3cOCW1EYoVK5ftnFFO/NQWFGkDqqQgUoAAWFCPxYUkQ"};
    // Key is o8cZ?nWv7h[jtY]*<H&CT<CL(f{zGz&Z6LK9o[EL with SENSITIVE hashing
    const std::string publicKey = "o8cZ?nWv7h[jtY]*<H&CT<CL(f{zGz&Z6LK9o[EL";
    // Sensitive
    //const std::string hashedPublicKey{"$argon2id$v=19$m=1048576,t=4,p=1$bBYEfas+MYYy9hsfND3rqg$NZXhVv8rF8CcE2ERg90L7p0chkrtRqP8+jXTWwfKhn0"};
    // Moderate
    //const std::string hashedPublicKey{"$argon2id$v=19$m=262144,t=3,p=1$O6vEofky6H6v9FdbAY+rXg$DK2w9VWV34N2DCepM5gCbZrmtqRXadgwxm9q6iAN/EE"};
    // Interactive
    //const std::string hashedPublicKey{"$argon2id$v=19$m=65536,t=2,p=1$+YTFJ9DZwiiX8c4866brIA$XJBjmDCmeGXAdh1PL/xfPHqd7Kpvile1rDUAefvWcHU"};
    const UserPrivileges privileges = UserPrivileges::ADMINISTRATOR;
 
    EXPECT_EQ(user.getMaximumHashedStringLength(), crypto_pwhash_STRBYTES);
    user.setIdentifier(id);
    EXPECT_NO_THROW(user.setName(name));
    EXPECT_NO_THROW(user.setEmail(email));
    EXPECT_NO_THROW(user.setHashedPassword(hashedPassword));
    EXPECT_NO_THROW(user.setPublicKey(publicKey));
    user.setPrivileges(privileges);

    User userCopy(user);
    EXPECT_EQ(userCopy.getIdentifier(), id);
    EXPECT_EQ(userCopy.getName(), name);
    EXPECT_EQ(userCopy.getEmail(), email);
    EXPECT_EQ(userCopy.getHashedPassword(), hashedPassword);
    EXPECT_EQ(userCopy.getPublicKey(), publicKey);
    EXPECT_EQ(userCopy.getPrivileges(), privileges);

    //std::cout << "Testing password..." << std::endl;
    EXPECT_TRUE(userCopy.doesPasswordMatch(password));
    //std::cout << "Testing pubilc key..." << std::endl;
    EXPECT_TRUE(userCopy.doesPublicKeyMatch(publicKey));
    //std::cout << "Testing fast tracks..." << std::endl;
    EXPECT_FALSE(userCopy.doesPasswordMatch(publicKey));
    EXPECT_FALSE(userCopy.doesPublicKeyMatch(password));
    EXPECT_TRUE(userCopy.doesPasswordMatch(password));
    EXPECT_TRUE(userCopy.doesPublicKeyMatch(publicKey));

    userCopy.clear();
    //std::cout << "Testing other" << std::endl;
    EXPECT_FALSE(userCopy.doesPasswordMatch(password));
    EXPECT_FALSE(userCopy.doesPublicKeyMatch(publicKey));

    auto json = user.toJSON();
    userCopy.fromJSON(json);
    EXPECT_EQ(userCopy.getIdentifier(), id);
    EXPECT_EQ(userCopy.getName(), name);
    EXPECT_EQ(userCopy.getEmail(), email);
    EXPECT_EQ(userCopy.getHashedPassword(), hashedPassword);
    EXPECT_EQ(userCopy.getPublicKey(), publicKey);
    EXPECT_EQ(userCopy.getPrivileges(), privileges);
    userCopy.clear();

    user.setIdentifier(id);
    auto message = user.toMessage();
    userCopy.fromMessage(message.c_str(), message.size());
    EXPECT_EQ(userCopy.getIdentifier(), id);
    EXPECT_EQ(userCopy.getName(), name);
    EXPECT_EQ(userCopy.getEmail(), email);
    EXPECT_EQ(userCopy.getHashedPassword(), hashedPassword);
    EXPECT_EQ(userCopy.getPublicKey(), publicKey);
    EXPECT_EQ(userCopy.getPrivileges(), privileges);


}

TEST(Messaging, SQLite3Authenticator)
{
    SQLite3Authenticator auth;
    const bool createIfDoesNotExist = true;
    std::string usersTable = "tables/users.sqlite3";
    std::string blacklistTable = "tables/blacklist.sqlite3";
    std::string whitelistTable = "tables/whitelist.sqlite3";
    EXPECT_FALSE(auth.haveUsersTable());
    EXPECT_FALSE(auth.haveWhitelistTable());
    EXPECT_FALSE(auth.haveBlacklistTable());

    EXPECT_NO_THROW(auth.openUsersTable(usersTable, createIfDoesNotExist));
    EXPECT_TRUE(auth.haveUsersTable());

    EXPECT_NO_THROW(auth.openBlacklistTable(blacklistTable,
                                            createIfDoesNotExist));
    EXPECT_TRUE(auth.haveBlacklistTable());

    EXPECT_NO_THROW(auth.openWhitelistTable(whitelistTable,
                                           createIfDoesNotExist));
    EXPECT_TRUE(auth.haveWhitelistTable());
 
    // Create users
    std::vector<User> users;
    for (int i = 0; i < 5; ++i)
    {
        std::string userName = "user" + std::to_string(i);
        std::string password = "password" + std::to_string(i);
        std::string email = userName + "@domain.com";
        Certificate::UserNameAndPassword plainText;
        plainText.setUserName(userName);
        plainText.setPassword(password);
        
        Certificate::Keys certificate;
        EXPECT_NO_THROW(certificate.create());

        // In real-life use a more secure method
        auto hashedPassword =
           plainText.getHashedPassword(Certificate::HashLevel::INTERACTIVE);
        // Build up the user
        User user;
        user.setName(plainText.getUserName());
        user.setEmail(email);
        user.setHashedPassword(hashedPassword);
        user.setPrivileges(UserPrivileges::READ_WRITE);
        // Add the user
        auth.addUser(user);
        // Give the user a public key
        auto publicKey = std::string(certificate.getPublicTextKey().data());
        user.setPublicKey(publicKey);
        auth.updateUser(user);
        // Save the credentials 
        users.push_back(user);

        std::string status, reason;
        std::tie(status, reason) = auth.isValid(plainText);
        EXPECT_EQ(status, auth.okayStatus());

        std::tie(status, reason) = auth.isValid(certificate);
        EXPECT_EQ(status, auth.okayStatus());
    }

    std::remove(usersTable.c_str());
    std::remove(blacklistTable.c_str());
    std::remove(whitelistTable.c_str());
}
*/

UMPS::MessageFormats::Pick makePickMessage() noexcept
{
    UMPS::MessageFormats::Pick pick;
    pick.setIdentifier(4043);
    pick.setTime(600);
    pick.setNetwork("UU"); 
    pick.setStation("NOQ");
    pick.setChannel("EHZ");
    pick.setLocationCode("01");
    pick.setPhaseHint("P");
    return pick;
}

void pub(std::shared_ptr<zmq::context_t> context,
         const Certificate::Keys serverCertificate)
{
    bool isAuthenticationServer = true;
    Certificate::UserNameAndPassword plainText; 
    plainText.setUserName("server");
    plainText.setPassword("password");

    UMPS::Messaging::PublisherSubscriber::Publisher publisher(context);
    //publisher.bind("tcp:// *:5555", isAuthenticationServer); // Strawhouse
    //publisher.bind("tcp:// *:5555", plainText, isAuthenticationServer); // Woodhouse
    publisher.bind("tcp://*:5555", serverCertificate); //  Stonehouse
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Define message to send
    auto pick = makePickMessage();
    // Send it
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //std::cout << "sending..." << std::endl;
    publisher.send(pick);
}

void sub(const Certificate::Keys serverCertificate)
{
    Certificate::Keys clientCertificate;
    clientCertificate.create();

    Certificate::UserNameAndPassword plainText;
    plainText.setUserName("client");
    plainText.setPassword("letMeIn");

    ZAPOptions zapOptions;
    zapOptions.setStonehouseClient(serverCertificate, clientCertificate);
   
    std::unique_ptr<UMPS::MessageFormats::IMessage> pickMessageType
        = std::make_unique<UMPS::MessageFormats::Pick> (); 
    UMPS::MessageFormats::Messages messageTypes;
    messageTypes.add(pickMessageType);

    UMPS::Messaging::PublisherSubscriber::SubscriberOptions options;
    options.setAddress("tcp://127.0.0.1:5555");
    options.setMessageTypes(messageTypes);
    options.setZAPOptions(zapOptions);

    bool isAuthenticationServer = false;
    UMPS::Messaging::PublisherSubscriber::Subscriber subscriber;
    //subscriber.connect("tcp://127.0.0.1:5555", isAuthenticationServer); // Grasslands/Strawhouse
    //subscriber.connect("tcp://127.0.0.1:5555", plainText, isAuthenticationServer); // Woodhouse
    //subscriber.connect("tcp://127.0.0.1:5555", serverCertificate, clientCertificate); // Stonehouse
    subscriber.initialize(options);
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    //std::cout << "getting" << std::endl;
    auto message = subscriber.receive();
    //std::cout << "done" << std::endl;

    //std::this_thread::sleep_for(std::chrono::seconds(3));
    auto pickMessage
        = static_unique_pointer_cast<UMPS::MessageFormats::Pick>
          (std::move(message));
    //std::cout << pickMessage->toJSON() << std::endl;
    auto pick = makePickMessage();
    EXPECT_NEAR(pickMessage->getTime(), pick.getTime(), 1.e-10);
    EXPECT_EQ(pickMessage->getIdentifier(),   pick.getIdentifier());
    EXPECT_EQ(pickMessage->getNetwork(),      pick.getNetwork());
    EXPECT_EQ(pickMessage->getStation(),      pick.getStation());
    EXPECT_EQ(pickMessage->getChannel(),      pick.getChannel());
    EXPECT_EQ(pickMessage->getLocationCode(), pick.getLocationCode());
    EXPECT_EQ(pickMessage->getPhaseHint(),    pick.getPhaseHint());
    EXPECT_EQ(pickMessage->getPolarity(),     pick.getPolarity());

}

TEST(Messaging, Authenticator)
{
    Certificate::Keys serverCertificate;
    serverCertificate.create();

    auto context = std::make_shared<zmq::context_t> (1);
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    Service auth(context, loggerPtr);

    // Have this thread start the authenticator
    std::thread t1(&Service::start, &auth);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    //auth.whitelist("127.0.0.1");
    //auth.start();

    auto publisherThread = std::thread(pub, context, serverCertificate);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::thread t3(sub, serverCertificate);
    std::this_thread::sleep_for(std::chrono::seconds(1));

sleep(1);
//std::cout << auth.isRunning() << std::endl;
//    std::cout << "killing it" << std::endl;
//    auth.stop();
sleep(1);
    publisherThread.join();
    t3.join();
    auth.stop();
    t1.join();
}


}
