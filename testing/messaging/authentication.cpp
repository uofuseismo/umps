#include <zmq.h>
#include "urts/messaging/authentication/certificate.hpp"
#include <gtest/gtest.h>
namespace
{

using namespace URTS::Messaging::Authentication;

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

    certificate.writeToTextFile("temp.key");
}

}
