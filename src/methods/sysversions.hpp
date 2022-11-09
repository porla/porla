#pragma once

#include <map>
#include <string>

#include "method.hpp"

namespace porla::Methods
{
    class SysVersions : public Method<json, std::map<std::string, std::string>>
    {
    protected:
        void Invoke(const json& req, WriteCb<std::map<std::string, std::string>> cb) override;
    };
}
