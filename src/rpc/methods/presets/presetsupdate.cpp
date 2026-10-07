#include "presetsupdate.hpp"

#include "../../../presets.hpp"

using porla::Rpc::Methods::Presets::PresetsUpdate;
using porla::Rpc::Methods::Presets::PresetsUpdateReq;
using porla::Rpc::Methods::Presets::PresetsUpdateRes;

PresetsUpdate::PresetsUpdate(porla::Presets& presets)
    : m_presets(presets)
{
}

void PresetsUpdate::Execute(const PresetsUpdateReq &req, ResponseWriterHandle cb)
{
    const auto preset = m_presets.Get(req.id);

    if (!preset.has_value())
    {
        return cb->Error(-1, "Preset not found", {{"id", req.id}});
    }

    if (req.flags.has_value() != req.flags_mask.has_value())
    {
        return cb->Error(-2, "'flags' and 'flags_mask' must be set together");
    }

    m_presets.Update(porla::Presets::Preset{
        .id = req.id,
        .name = req.name,
        .is_default = req.is_default.value_or(preset->is_default),
        .category = req.category,
        .download_limit = req.download_limit,
        .flags = req.flags,
        .flags_mask = req.flags_mask,
        .max_connections = req.max_connections,
        .max_uploads = req.max_uploads,
        .metadata = req.metadata,
        .session_id = req.session_id,
        .save_path = req.save_path,
        .storage_mode = req.storage_mode,
        .tags = req.tags,
        .upload_limit = req.upload_limit
    });

    cb->Ok(PresetsUpdateRes{});
}
