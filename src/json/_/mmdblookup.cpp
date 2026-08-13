#include "../all.hpp"

#include "../../methods/mmdb/mmdblookup_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Mmdb
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        MmdbLookupReq,
        values)

    NLOHMANN_JSONIFY_ALL_THINGS(
        MmdbLookupRes,
        results)
}
