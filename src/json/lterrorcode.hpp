#pragma once

#include <libtorrent/error_code.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace boost::system
{
    static void from_json(const json& j, boost::system::error_code& ec)
    {
    }

    static void to_json(json& j, const boost::system::error_code& ec)
    {
        if (ec)
        {
            j = {
                {"message", ec.message()},
                {"value", ec.value()}
            };
        }
    }
}
