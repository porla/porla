#pragma once

#include <map>
#include <string>

#include "method.hpp"

namespace porla::Methods
{
    template <bool SSL> class SysVersions : public Method<json, std::map<std::string, std::string>, SSL>
    {
    protected:
        void Invoke(const json& req, WriteCb<std::map<std::string, std::string>, SSL> cb) override;
    };
}
