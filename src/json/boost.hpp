#pragma once

#include <nlohmann/json.hpp>

namespace boost::system
{
    class error_code;

    inline void from_json(const nlohmann::json& json, error_code& ec) {}
    void to_json(nlohmann::json& json, const error_code& ec);
}
