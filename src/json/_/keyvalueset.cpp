#include "../all.hpp"

#include "../../methods/keyvalueset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueSetReq,
        values)

    void to_json(nlohmann::json& json, const KeyValueSetRes& res)
    {
        json = {};
    }
}
