#include "presetslist.hpp"

#include "../../data/models/presets.hpp"

using porla::Methods::Presets::PresetsList;
using porla::Methods::Presets::PresetsListReq;
using porla::Methods::Presets::PresetsListRes;

PresetsList::PresetsList(sqlite3* db)
    : m_db(db)
{
}

void PresetsList::Invoke(const PresetsListReq& req, WriteCb<PresetsListRes> cb)
{
    std::vector<PresetsListRes::ListItem> presets;

    Data::Models::Presets::ForEach(
        m_db,
        [&presets](const auto& preset)
        {
            presets.emplace_back(PresetsListRes::ListItem{
                .id         = preset.id,
                .name       = preset.name,
                .is_default = preset.is_default,
                .metadata   = preset.metadata.has_value()
                    ? preset.metadata.value()
                    : std::map<std::string, nlohmann::json>()
            });
        });

    cb.Ok(PresetsListRes{
        .presets = presets
    });
}
