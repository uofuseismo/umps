#include <iostream>
#include <vector>
#include <array>
#include <cassert>
#include <fstream>
#include <zmq.h>
#include <sodium.h>
#include "urts/messaging/authentication/generateKeyPair.hpp"

/// Generate a key pair
std::pair<std::vector<char>, std::vector<char>>
    URTS::Messaging::Authentication::generateKeyPair()
{
    // Have NaCl create the keys
    std::array<unsigned char, crypto_box_PUBLICKEYBYTES> publicKey;
    std::array<unsigned char, crypto_box_SECRETKEYBYTES> privateKey;
    int rc = crypto_box_keypair(publicKey.data(), privateKey.data());
    if (rc != 0)
    {   
        throw std::runtime_error("Failed to make crypto box keypair");
    }   
    if (publicKey.size()%4 != 0)
    {   
        throw std::runtime_error("Public key length must be divisible by 4");
    }   
    if (privateKey.size()%4 != 0)
    {   
        throw std::runtime_error("Private key length must be divisible by 4");
    }   
    // Encode the keys
    auto encodedLength = static_cast<size_t> (1.25*publicKey.size()) + 1;
    std::vector<char> publicKeyEncoded(encodedLength, '\0');
    auto retval = zmq_z85_encode(publicKeyEncoded.data(),
                                 publicKey.data(), publicKey.size());
    if (retval == nullptr)
    {
        throw std::runtime_error("Failed to encode public key");
    }

    encodedLength = static_cast<size_t> (1.25*privateKey.size()) + 1;
    std::vector<char> privateKeyEncoded(encodedLength, '\0');
    retval = zmq_z85_encode(privateKeyEncoded.data(),
                            privateKey.data(), privateKey.size());
    if (retval == nullptr)
    {
        throw std::runtime_error("Failed to encode private key");
    }
    // Return them
    return std::pair(publicKeyEncoded, privateKeyEncoded);
}
