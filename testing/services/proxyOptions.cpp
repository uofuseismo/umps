#include <string>
#include "umps/proxyServices/proxyOptions.hpp"
#include "umps/authentication/zapOptions.hpp"
#include "umps/messaging/routerDealer/proxyOptions.hpp"
#include <gtest/gtest.h>

namespace
{

using namespace UMPS::ProxyServices;
namespace UAuth = UMPS::Authentication;
namespace RouterDealer = UMPS::Messaging::RouterDealer;

TEST(ProxyServices, ProxyOptions)
{
    const std::string frontendAddress = "tcp://127.0.0.1:5555";
    const std::string backendAddress = "tcp://127.0.0.2:5556";
    const std::string name = "TestPacketCacheService";
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
    EXPECT_EQ(options.getProxyOptions().getFrontendHighWaterMark(), 8192);//2000);
    EXPECT_EQ(options.getProxyOptions().getBackendHighWaterMark(),  0);//1000);
    EXPECT_EQ(options.getProxyOptions().getZAPOptions().getSecurityLevel(),
              UAuth::SecurityLevel::GRASSLANDS);
}

}
