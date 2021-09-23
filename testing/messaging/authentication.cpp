#include <zmq.h>
#include "umps/messaging/authentication/certificate.hpp"
#include "umps/messaging/authentication/authenticator.hpp"
#include "umps/messaging/authentication/threadAuthenticator.hpp"
#include "umps/logging/stdout.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace UMPS::Messaging::Authentication;

TEST(Messaging, Certificate)
{
    Certificate certificate;
    const std::string metadata = "Test metadata";
    EXPECT_NO_THROW(certificate.create());
    EXPECT_TRUE(certificate.haveKeyPair());
    certificate.setMetadata(metadata);
    EXPECT_EQ(metadata, certificate.getMetadata());
    auto publicTextKey  = certificate.getPublicTextKey();
    auto privateTextKey = certificate.getPrivateTextKey(); 
    auto publicKey  = certificate.getPublicKey();
    auto privateKey = certificate.getPrivateKey();

    Certificate fromText;
    fromText.setPublicKey(publicTextKey);
    fromText.setPrivateKey(privateTextKey);
    EXPECT_EQ(fromText.getPublicKey(),  certificate.getPublicKey());
    EXPECT_EQ(fromText.getPrivateKey(), certificate.getPrivateKey());
  
    Certificate fromBinary;
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
 
    Certificate fromFile;
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

TEST(Messaging, Authenticator)
{
    UMPS::Logging::StdOut logger;
    logger.setLevel(UMPS::Logging::Level::DEBUG);
    std::shared_ptr<UMPS::Logging::ILog> loggerPtr
        = std::make_shared<UMPS::Logging::StdOut> (logger);
    Authenticator auth(loggerPtr);
    auth.start();
    auth.stop();
}

}
