#include "../all.hpp"

#include "../../methods/presets/presetsadd_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsAddReq,
        name)

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsAddRes,
        id)
}
