#pragma once

#include <map>
#include <string>

#include "../../typedmethod.hpp"

namespace porla::Rpc::Methods::Sys
{
    class SysVersions : public TypedMethod<nlohmann::json, std::map<std::string, nlohmann::json>>
    {
    protected:
        void Execute(const nlohmann::json& req, ResponseWriterHandle cb) override;
    };
}
