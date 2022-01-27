#include <string>
#include "umps/proxyBroadcasts/proxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/xPublisherXSubscriber/proxyOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyBroadcasts;
namespace UAuth = UMPS::Authentication;
namespace UXPubXSub = UMPS::Messaging::XPublisherXSubscriber;

TEST(ProxyBroadcasts, ProxyOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const std::string backendAddress = "tcp://127.0.0.2:5556";
    const std::string name = "TestPacketBroadcast";
    const int frontendHWM = 100;
    const int backendHWM = 200;
    UAuth::ZAPOptions zapOptions;
    zapOptions.setStrawhouseServer();
    ProxyOptions options;
    options.setName(name);
    options.setFrontendAddress(frontendAddress);
    options.setFrontendHighWaterMark(frontendHWM);
    options.setBackendAddress(backendAddress);
    options.setBackendHighWaterMark(backendHWM);
    options.setZAPOptions(zapOptions);
  
    ProxyOptions optionsCopy(options);

    EXPECT_EQ(optionsCopy.getName(), name);
    EXPECT_EQ(optionsCopy.getProxyOptions().getFrontendAddress(),
              frontendAddress);
    EXPECT_EQ(optionsCopy.getProxyOptions().getBackendAddress(),
              backendAddress);
    EXPECT_EQ(optionsCopy.getProxyOptions().getFrontendHighWaterMark(),
              frontendHWM);
    EXPECT_EQ(optionsCopy.getProxyOptions().getBackendHighWaterMark(),
              backendHWM);
    EXPECT_EQ(optionsCopy.getProxyOptions().getZAPOptions().getSecurityLevel(),
              zapOptions.getSecurityLevel());
   
    options.clear();
    EXPECT_EQ(options.getProxyOptions().getFrontendHighWaterMark(), 4096);
    EXPECT_EQ(options.getProxyOptions().getBackendHighWaterMark(),  4096);
    EXPECT_EQ(options.getProxyOptions().getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::GRASSLANDS);
}

}
