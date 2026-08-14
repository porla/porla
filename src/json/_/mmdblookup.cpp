#include "../all.hpp"

#include "../../rpc/methods/mmdb/mmdblookup_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Mmdb
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        MmdbLookupReq,
        values)

    NLOHMANN_JSONIFY_ALL_THINGS(
        MmdbLookupRes,
        results)
}
