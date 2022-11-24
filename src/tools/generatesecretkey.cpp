#include "generatesecretkey.hpp"

#include "../utils/secretkey.hpp"

int porla::Tools::GenerateSecretKey(int argc, char **argv, std::unique_ptr<porla::Config> cfg)
{
    const std::string key = porla::Utils::SecretKey::New();
    printf("%s\n", key.c_str());
    return 0;
}
