#include "authtoken.hpp"

#include <jwt-cpp/jwt.h>

#include "../config.hpp"

int porla::Tools::AuthToken(int argc, char **argv, std::unique_ptr<porla::Config> cfg)
{
    auto token = jwt::create()
            .set_issuer("porla")
            .set_issued_at(std::chrono::system_clock::now())
            .set_type("JWS")
            .sign(jwt::algorithm::hs256(cfg->secret_key));

    printf("%s\n", token.c_str());

    return 0;
}
