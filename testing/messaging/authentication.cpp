#include <zmq.hpp>
#include <thread>
#include "umps/messaging/authentication/certificate/keys.hpp"
#include "umps/messaging/authentication/certificate/userNameAndPassword.hpp"
#include "umps/messaging/authentication/service.hpp"
#include "umps/messaging/publisherSubscriber/publisher.hpp"
#include "umps/messaging/publisherSubscriber/subscriber.hpp"
#include "umps/messageFormats/pick.hpp"
#include "umps/logging/stdout.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace UMPS::Messaging::Authentication;

TEST(Messaging, CertificateUserNameAndPassword)
{
    const std::string userName = "user";
    const std::string password = "password";
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

void pub(std::shared_ptr<zmq::context_t> context,
         const Certificate::Keys serverCertificate)
{
    bool isAuthenticationServer = true;
    Certificate::UserNameAndPassword plainText; 
    plainText.setUserName("server");
    plainText.setPassword("password");

    UMPS::Messaging::PublisherSubscriber::Publisher publisher(context);
    //publisher.bind("tcp://*:5555", isAuthenticationServer); // Strawhouse
    //publisher.bind("tcp://*:5555", plainText, isAuthenticationServer); // Woodhouse
    publisher.bind("tcp://*:5555", serverCertificate); //  Stonehouse
    std::this_thread::sleep_for(std::chrono::seconds(1));
    // Define message to send
    UMPS::MessageFormats::Pick pick;
    pick.setIdentifier(4043);
    pick.setTime(600);
    pick.setNetwork("UU"); 
    pick.setStation("NOQ");
    pick.setChannel("EHZ");
    pick.setLocationCode("01");
    pick.setPhaseHint("P");
    // Send it
    std::this_thread::sleep_for(std::chrono::seconds(1));
std::cout << "sending..." << std::endl;
    publisher.send(pick);

}

void sub(const Certificate::Keys serverCertificate)
{
    Certificate::Keys clientCertificate;
    clientCertificate.create();

    Certificate::UserNameAndPassword plainText;
    plainText.setUserName("client");
    plainText.setPassword("letMeIn");

    bool isAuthenticationServer = false;
    UMPS::Messaging::PublisherSubscriber::Subscriber subscriber;
    //subscriber.connect("tcp://127.0.0.1:5555", isAuthenticationServer); // Grasslands/Strawhouse
    //subscriber.connect("tcp://127.0.0.1:5555", plainText, isAuthenticationServer); // Woodhouse
    subscriber.connect("tcp://127.0.0.1:5555", serverCertificate, clientCertificate); // Stonehouse
    std::unique_ptr<UMPS::MessageFormats::IMessage> pickMessageType
        = std::make_unique<UMPS::MessageFormats::Pick> ();
    subscriber.addSubscription(pickMessageType);
//    std::this_thread::sleep_for(std::chrono::seconds(1));
std::cout << "getting" << std::endl;
    auto message = subscriber.receive();
std::cout << "done" << std::endl;

    //std::this_thread::sleep_for(std::chrono::seconds(3));

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
std::cout << auth.isRunning() << std::endl;
    std::cout << "killing it" << std::endl;
//    auth.stop();
sleep(1);
    publisherThread.join();
    t3.join();
    auth.stop();
    t1.join();
}

}
