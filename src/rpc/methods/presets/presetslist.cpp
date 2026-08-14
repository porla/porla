#include "presetslist.hpp"

#include "../../../data/models/presets.hpp"

using porla::Rpc::Methods::Presets::PresetsList;
using porla::Rpc::Methods::Presets::PresetsListReq;
using porla::Rpc::Methods::Presets::PresetsListRes;

PresetsList::PresetsList(sqlite3* db)
    : m_db(db)
{
}

void PresetsList::Execute(const PresetsListReq& req, ResponseWriterHandle cb)
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

    cb->Ok(PresetsListRes{
        .presets = presets
    });
}
