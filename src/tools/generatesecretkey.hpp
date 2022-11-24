#pragma once

#include <memory>

namespace porla { class Config; }

namespace porla::Tools
{
    int GenerateSecretKey(int argc, char* argv[], std::unique_ptr<porla::Config> cfg);
}
