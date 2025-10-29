#include "../all.hpp"

#include "../../methods/keyvalueget_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueGetReq,
        keys)

    NLOHMANN_JSONIFY_ALL_THINGS(
        KeyValueGetRes,
        values)
}
