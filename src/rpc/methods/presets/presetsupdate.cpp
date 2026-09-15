#include "presetsupdate.hpp"

#include "../../../data/models/presets.hpp"

using porla::Rpc::Methods::Presets::PresetsUpdate;
using porla::Rpc::Methods::Presets::PresetsUpdateReq;
using porla::Rpc::Methods::Presets::PresetsUpdateRes;

PresetsUpdate::PresetsUpdate(sqlite3 *db)
    : m_db(db)
{
}

void PresetsUpdate::Execute(const PresetsUpdateReq &req, ResponseWriterHandle cb)
{
    const auto preset = Data::Models::Presets::GetById(m_db, req.id);

    if (!preset.has_value())
    {
        return cb->Error(-1, "Preset not found", {{"id", req.id}});
    }

    Data::Models::Presets::Update(
        m_db,
        Data::Models::Presets::Preset{
            .id = req.id,
            .name = req.name,
            .is_default = req.is_default.value_or(preset->is_default),
            .category = req.category,
            .download_limit = req.download_limit,
            .max_connections = req.max_connections,
            .max_uploads = req.max_uploads,
            .metadata = req.metadata,
            .session_id = req.session_id,
            .save_path = req.save_path,
            .storage_mode = req.storage_mode,
            .tags = req.tags,
            .upload_limit = req.upload_limit});

    cb->Ok(PresetsUpdateRes{});
}
