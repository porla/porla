#include "../all.hpp"

#include "../../rpc/methods/presets/presetslist_reqres.hpp"
#include "../utils.hpp"

namespace porla::Rpc::Methods::Presets
{
    void from_json(const nlohmann::json& json, PresetsListReq& req)
    {
    }

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsListRes::ListItem,
        id,
        name,
        is_default,
        metadata);

    NLOHMANN_JSONIFY_ALL_THINGS(
        PresetsListRes,
        presets);
}
