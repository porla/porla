#include "../all.hpp"

#include "../../rpc/methods/presets/presetsadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsAddReq,
        name)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsAddRes,
        id)
}
