#include "presetsremove.hpp"

#include "../../../presets.hpp"

using porla::Rpc::Methods::Presets::PresetsRemove;
using porla::Rpc::Methods::Presets::PresetsRemoveReq;
using porla::Rpc::Methods::Presets::PresetsRemoveRes;

PresetsRemove::PresetsRemove(porla::Presets& presets)
    : m_presets(presets)
{
}

void PresetsRemove::Execute(const PresetsRemoveReq& req, ResponseWriterHandle cb)
{
    const auto preset = m_presets.Get(req.id);

    if (!preset.has_value())
    {
        return cb->Error(-1, "Preset not found");
    }

    m_presets.Remove(preset->id);

    cb->Ok(PresetsRemoveRes{});
}
