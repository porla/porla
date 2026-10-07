#include "presetslist.hpp"

#include "../../../presets.hpp"

using porla::Rpc::Methods::Presets::PresetsList;
using porla::Rpc::Methods::Presets::PresetsListReq;
using porla::Rpc::Methods::Presets::PresetsListRes;

PresetsList::PresetsList(porla::Presets& presets)
    : m_presets(presets)
{
}

void PresetsList::Execute(const PresetsListReq& req, ResponseWriterHandle cb)
{
    std::vector<PresetsListRes::ListItem> presets;

    for (const auto& preset : m_presets.All())
    {
        presets.emplace_back(PresetsListRes::ListItem{
            .id         = preset.id,
            .name       = preset.name,
            .is_default = preset.is_default,
            .metadata   = preset.metadata.has_value()
                ? preset.metadata.value()
                : std::map<std::string, nlohmann::json>()
        });
    }

    cb->Ok(PresetsListRes{
        .presets = presets
    });
}
