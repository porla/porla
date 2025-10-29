#include "../all.hpp"

#include "../../methods/presets/presetslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Methods::Presets
{
    void from_json(const nlohmann::json& json, PresetsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsListRes::ListItem,
        id,
        name);

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsListRes,
        presets);
}
