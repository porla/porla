#include "../all.hpp"

#include "../../rpc/methods/kv/keyvalueset_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Kv
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueSetReq,
        values)

    void to_json(nlohmann::json& json, const KeyValueSetRes& res)
    {
        json = {};
    }
}
