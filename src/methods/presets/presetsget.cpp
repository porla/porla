#include "presetsget.hpp"

#include "../../data/models/presets.hpp"

using porla::Methods::Presets::PresetsGet;
using porla::Methods::Presets::PresetsGetReq;
using porla::Methods::Presets::PresetsGetRes;

PresetsGet::PresetsGet(sqlite3 *db)
    : m_db(db)
{
}

void PresetsGet::Invoke(const PresetsGetReq &req, WriteCb<PresetsGetRes> cb)
{
    const auto preset = Data::Models::Presets::GetById(m_db, req.id);

    if (!preset.has_value())
    {
        return cb.Error(-1, "Preset not found");
    }

    cb.Ok(PresetsGetRes{
        .id = preset->id,
        .name = preset->name,
        .category = preset->category,
        .download_limit = preset->download_limit,
        .max_connections = preset->max_connections,
        .max_uploads = preset->max_uploads,
        .metadata = preset->metadata,
        .session_id = preset->session_id,
        .save_path = preset->save_path,
        .storage_mode = preset->storage_mode,
        .tags = preset->tags,
        .upload_limit = preset->upload_limit
    });
}
