#include "../all.hpp"

#include "../../methods/presets/presetsremove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsRemoveReq,
        id)

    void to_json(nlohmann::json& j, const PresetsRemoveRes& res)
    {
        j = {};
    }
}
