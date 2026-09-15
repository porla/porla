#include "../all.hpp"

#include "../../rpc/methods/presets/presetsremove_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsRemoveReq,
        id)

    void to_json(nlohmann::json& j, const PresetsRemoveRes& res)
    {
        j = {};
    }
}
