#include <iostream>
#include <string>
#include <array>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <ctime>
#include <chrono>
#include <zmq.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "umps/messaging/authentication/certificate.hpp"

/// N.B. A lot of the implementation ideas for this class are from zeromq's 
///      python interface.  This is a port of the `high-level' C bindings.
///      My issue with the the `high-level' C bindings was they didn't contain
///      a custom callback for validation in the case of wanting to use a
///      database.  The `high-level' C++ bindings have an implementation
///      as well but are also missing the callback.  Also, it is unclear if
///      that library is being maintained.  In the future, the cppzmq header
///      only library may bring this functionality into their tool which would
///      deprecate all of this - and that's fine.  For more read:
///      https://github.com/zeromq/pyzmq/blob/main/zmq/auth/certs.py
///      Ben Baker (September 2021)
using namespace UMPS::Messaging::Authentication;

namespace
{
std::string createTimeStamp()
{
    typedef std::chrono::duration<int,
        std::ratio_multiply<std::chrono::hours::period,
        std::ratio<24> >::type> days;
    auto now = std::chrono::system_clock::now();
    std::chrono::system_clock::duration tp = now.time_since_epoch();
    auto d = std::chrono::duration_cast<days>(tp);
    tp -= d;
    auto h = std::chrono::duration_cast<std::chrono::hours> (tp);
    tp -= h;
    auto m = std::chrono::duration_cast<std::chrono::minutes> (tp);
    tp -= m;
    auto s = duration_cast<std::chrono::seconds> (tp);
    tp -= s;
    //std::cout << d.count() << "d " << h.count() << ':'
    //          << m.count() << ':' << s.count();
    time_t tt = std::chrono::system_clock::to_time_t(now);
    tm utc_tm = *gmtime(&tt);
    char cDate[32];
    std::fill(cDate, cDate + 32, '\0');
    sprintf(cDate, "%04d-%02d-%02d %02d:%02d:%02d",
             utc_tm.tm_year + 1900,
             utc_tm.tm_mon + 1,
             utc_tm.tm_mday,
             utc_tm.tm_hour,
             utc_tm.tm_min,
             utc_tm.tm_sec);
    return std::string(cDate);
}

void createRootDirectoryFromFileName(const std::string &fileName)
{
    auto path = std::filesystem::path(fileName).parent_path();
    if (!path.empty() && !std::filesystem::exists(path))
    {
        std::filesystem::create_directories(path);
        if (!std::filesystem::exists(path))
        {
            throw std::invalid_argument("Failed to create path: "
                                      + std::string(path));
        }
    }
}

}

class Certificate::CertificateImpl
{
public:
    std::string mMetadata;
    std::array<uint8_t, 32> mPublicKey;
    std::array<uint8_t, 32> mPrivateKey;
    std::array<char, 41> mPublicText;
    std::array<char, 41> mPrivateText;
    bool mHavePublicKey = false;
    bool mHavePrivateKey = false;
};

/// C'tor
Certificate::Certificate() :
    pImpl(std::make_unique<CertificateImpl> ())
{
}

/// Copy c'tor
Certificate::Certificate(const Certificate &certificate)
{
    *this = certificate;
}

/// Move c'tor
Certificate::Certificate(Certificate &&certificate) noexcept
{
    *this = std::move(certificate);
}

/// Copy assignment
Certificate& Certificate::operator=(const Certificate &certificate)
{
    if (&certificate == this){return *this;}
    pImpl = std::make_unique<CertificateImpl> (*certificate.pImpl);
    return *this;
} 

/// Move assignment
Certificate& Certificate::operator=(Certificate &&certificate) noexcept
{
    if (&certificate == this){return *this;}
    pImpl = std::move(certificate.pImpl);
    return *this;
}

/// Destructor
Certificate::~Certificate() = default;

/// Clear
void Certificate::clear() noexcept
{
    pImpl->mMetadata.clear();
    pImpl->mHavePublicKey = false;
    pImpl->mHavePrivateKey = false;
}

/// Creates a keypair
void Certificate::create()
{
    std::array<char, 41> publicText; 
    std::array<char, 41> privateText;
    auto rc = zmq_curve_keypair(publicText.data(), privateText.data());
    if (rc != 0)
    {
        throw std::runtime_error("ZMQ failed to generate keypair with error: "
                               + std::to_string(rc));
    }
    setPair(publicText, privateText);
}

/// Sets the public key
void Certificate::setPublicKey(const std::array<uint8_t, 32> &key)
{
    pImpl->mHavePublicKey = false;
    pImpl->mPublicKey = key;
    auto result = zmq_z85_encode(pImpl->mPublicText.data(),
                                 pImpl->mPublicKey.data(),
                                 pImpl->mPublicKey.size());
    if (result == nullptr)
    { 
        throw std::runtime_error("Failed to encode public key");
    }
    pImpl->mHavePublicKey = true;
}

void Certificate::setPublicKey(const std::array<char, 41> &key)
{
    pImpl->mHavePublicKey = false;
    pImpl->mPublicText = key;
    std::array<char, 42> workKey;
    std::fill(workKey.begin(), workKey.end(), '\0');
    std::copy(key.begin(), key.end(), workKey.begin());
    auto result = zmq_z85_decode(pImpl->mPublicKey.data(),
                                 workKey.data());
    if (result == nullptr)
    {
        throw std::runtime_error("Failed to decode public key");
    }
    pImpl->mHavePublicKey = true;
}

std::array<uint8_t, 32> Certificate::getPublicKey() const
{
    if (!havePublicKey()){throw std::runtime_error("Public key not set");}
    return pImpl->mPublicKey;
}

std::array<char, 41> Certificate::getPublicTextKey() const
{
    if (!havePublicKey()){throw std::runtime_error("Public key not set");}
    return pImpl->mPublicText;
}

bool Certificate::havePublicKey() const noexcept
{
    return pImpl->mHavePublicKey;
}

/// Sets the private key
void Certificate::setPrivateKey(const std::array<uint8_t, 32> &key)
{
    pImpl->mHavePrivateKey = false;
    pImpl->mPrivateKey = key;
    auto result = zmq_z85_encode(pImpl->mPrivateText.data(),
                                 pImpl->mPrivateKey.data(),
                                 pImpl->mPrivateKey.size());
    if (result == nullptr)
    {
        throw std::runtime_error("Failed to encode private key");
    }
    pImpl->mHavePrivateKey = true;
}

void Certificate::setPrivateKey(const std::array<char, 41> &key)
{
    pImpl->mHavePrivateKey = false;
    pImpl->mPrivateText = key;
    std::array<char, 42> workKey;
    std::fill(workKey.begin(), workKey.end(), '\0');
    std::copy(key.begin(), key.end(), workKey.begin());
    auto result = zmq_z85_decode(pImpl->mPrivateKey.data(),
                                 workKey.data());
    if (result == nullptr)
    {
        throw std::runtime_error("Failed to decode private key");
    }
    pImpl->mHavePrivateKey = true;
}

std::array<uint8_t, 32> Certificate::getPrivateKey() const
{
    if (!havePrivateKey()){throw std::runtime_error("Private key not set");}
    return pImpl->mPrivateKey;
}

std::array<char, 41> Certificate::getPrivateTextKey() const
{
    if (!havePrivateKey()){throw std::runtime_error("Private key not set");}
    return pImpl->mPrivateText;
}

bool Certificate::havePrivateKey() const noexcept
{
    return pImpl->mHavePrivateKey;
}

/// Sets a keypair
void Certificate::setPair(const std::array<uint8_t, 32> &publicKey,
                          const std::array<uint8_t, 32> &privateKey)
{
    setPublicKey(publicKey);
    setPrivateKey(privateKey);
}

/// Sets a keypair 
void Certificate::setPair(const std::array<char, 41> &publicText,
                          const std::array<char, 41> &privateText)
{
    setPublicKey(publicText);
    setPrivateKey(privateText);
}

/// Have keypair?
bool Certificate::haveKeyPair() const noexcept
{
    return (havePublicKey() && havePrivateKey());
}

/// Metadata
void Certificate::setMetadata(const std::string &metadata) noexcept
{
    pImpl->mMetadata = metadata;
}

std::string Certificate::getMetadata() const noexcept
{
    return pImpl->mMetadata;
}

/// Write public key
void Certificate::writePublicKeyToTextFile(const std::string &fileName) const
{
    if (!havePublicKey())
    {
        throw std::runtime_error("Public key not set");
    }
    createRootDirectoryFromFileName(fileName);
    auto timeStamp = "#   **** Generated on " + createTimeStamp()
                   + " UTC by URMS ****";
    const std::string preamble =
R"""(#   ZeroMQ CURVE Public Certificate
#   Exchange securely, or use a secure mechanism to verify the contents
#   of this file after exchange.  It is recommended to store public your
#   certificates in your ${HOME}/.curve home directory.
)""";
    std::fstream outfl(fileName, std::ios::out); 
    outfl << timeStamp << std::endl;
    outfl << preamble << std::endl;
    auto metadata = getMetadata();
    if (!metadata.empty())
    {
        outfl << "metadata" << std::endl;
        outfl << "    name = " << '"' << getMetadata() << '"' << std::endl;
    }
    outfl << "curve" << std::endl;
    outfl << "    public-key = " << '"'
          << std::string(getPublicTextKey().data()) << '"' << std::endl;
    outfl.close();
}

/// Write private key
void Certificate::writePrivateKeyToTextFile(const std::string &fileName) const
{
    if (!havePrivateKey())
    {
        throw std::runtime_error("Private key not set");
    }
    createRootDirectoryFromFileName(fileName);
    auto timeStamp = "#   **** Generated on " + createTimeStamp()
                   + " UTC by URMS ****";
    const std::string preamble =
R"""(#   ZeroMQ CURVE **Secret** Certificate
#   DO NOT PROVIDE THIS FILE TO OTHER USERS nor change its permissions.
)""";
    std::fstream outfl(fileName, std::ios::out);
    outfl << timeStamp << std::endl;
    outfl << preamble << std::endl;
    auto metadata = getMetadata();
    if (!metadata.empty())
    {
        outfl << "metadata" << std::endl;
        outfl << "    name = " << '"' << getMetadata() << '"' << std::endl;
    }
    outfl << "curve" << std::endl;
    if (havePrivateKey())
    {
        outfl << "    secret-key = " << '"'
              << std::string(getPrivateTextKey().data()) << '"' << std::endl;
    }
    outfl.close();
}

/// Read a public key
void Certificate::loadFromTextFile(const std::string &fileName)
{
    if (!std::filesystem::exists(fileName))
    {
        throw std::invalid_argument("Public key file: " + fileName
                                  + " does not exist");
    }
    // Parse the file
    std::ifstream infl(fileName);
    if (infl.is_open())
    {
        std::string line;
        std::string publicKey;
        std::string privateKey;
        std::string metadata;
        while (std::getline(infl, line))
        {
            // Trim blank space off beginning and end
            boost::algorithm::trim(line); 
            if (line.empty()){continue;} // Empty line
            if (line[0] == '#'){continue;} // Skip comments
            // Looking for something of the form: variable = "stuff"
            auto foundEquals = line.find("="); // First equality
            if (foundEquals == std::string::npos){continue;}
            auto foundQuote = line.find('"'); // First quote
            // First quote should come after first equality
            if (foundQuote <= foundEquals){continue;}
            auto foundLastQuote = line.find_last_of('"');
            if (foundQuote >= foundLastQuote - 1){continue;} // Misformatted
            auto length = foundLastQuote - 2 - foundQuote + 1;
            // Data ends with a quote
            if (line.back() != '"'){continue;}
            // Okay - this is parsible - what is it?
            auto foundMetadata = line.find("name");
            if (foundMetadata == 0)
            {
                metadata = line.substr(foundQuote + 1, length);
                continue;
            }
            auto foundPublicKey = line.find("public-key");
            if (foundPublicKey == 0)
            {
                publicKey = line.substr(foundQuote + 1, length);
                continue;
            }
            auto foundPrivateKey = line.find("secret-key");
            if (foundPrivateKey == 0)
            {
                privateKey = line.substr(foundQuote + 1, length);
                continue;
            }
            continue; 
        }
        infl.close();
        if (!metadata.empty()){setMetadata(metadata);}
        if (!publicKey.empty())
        {
            if (publicKey.size() == 40)
            {
                std::array<char, 41> key;
                std::copy(publicKey.begin(), publicKey.end(), key.begin());
                key.back() = '\0';
                setPublicKey(key);
            }
            else
            {
                std::cerr << "Public key is wrong size" << std::endl;
            }
        }
        if (!privateKey.empty())
        {
            if (privateKey.size() == 40)
            {
                std::array<char, 41> key;
                std::copy(privateKey.begin(), privateKey.end(), key.begin());
                key.back() = '\0';
                setPrivateKey(key);
            }
            else
            {
                std::cerr << "Private key is wrong size" << std::endl;
            }
        }
    }
    else
    {
        throw std::runtime_error("Failed to open file");
    }
}

/*
/// Get public/private keys in text form
std::array<char, 41> Certificate::getPublicTextKey() const
{
    if (!haveCertificate()){throw std::runtime_error("Keys not set");}
    return pImpl->mPublicTextKey;
}

std::array<char, 41> Certificate::getPrivateTextKey() const
{
    if (!haveCertificate()){throw std::runtime_error("Keys not set");}
    return pImpl->mPrivateTextKey;
}

/// Get public/private keys in hex form
std::array<char, 32> Certificate::getPublicKey() const
{
    if (!haveCertificate()){throw std::runtime_error("Keys not set");}
    return pImpl->mPublicKey;
} 

std::array<char, 32> Certificate::getPrivateKey() const
{
    if (!haveCertificate()){throw std::runtime_error("Keys not set");}
    return pImpl->mPrivateKey;
}

/// Load the certificates from a text file
void Certificate::loadFromFile(const std::string &fileName)
{
    if (!std::filesystem::exists(fileName))
    {
        throw std::invalid_argument("Certificate file: " + fileName
                                  + " does not exist");
    }
}
*/
