#include "../all.hpp"

#include <boost/system/error_code.hpp>

namespace boost::system
{
    void to_json(nlohmann::json& json, const boost::system::error_code& ec)
    {
        if (ec)
        {
            json = {
                {"message", ec.message()},
                {"value", ec.value()}
            };
        }
    }
}
