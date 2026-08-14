#pragma once

#include <string>

namespace porla::Rpc::Methods::WebUI
{
    struct WebUIInstallReq
    {
        std::string version;
    };

    struct WebUIInstallRes
    {
    };
}