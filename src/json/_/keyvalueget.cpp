#include "../all.hpp"

#include "../../rpc/methods/kv/keyvalueget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Kv
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueGetReq,
        keys)

    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueGetRes,
        values)
}
