#include "secretkey.hpp"

#include <sodium.h>

using porla::Utils::SecretKey;

std::string SecretKey::New(int length)
{
    static std::string AllowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::string key;
    key.resize(length);

    for (char& c : key)
    {
        c = AllowedChars[randombytes_uniform(AllowedChars.size())];
    }

    return key;
}
