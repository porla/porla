#include "presetsupdate.hpp"

#include <boost/log/trivial.hpp>

#include "../../data/models/presets.hpp"

using porla::Methods::Presets::PresetsUpdate;
using porla::Methods::Presets::PresetsUpdateReq;
using porla::Methods::Presets::PresetsUpdateRes;

PresetsUpdate::PresetsUpdate(sqlite3 *db)
    : m_db(db)
{
}

void PresetsUpdate::Invoke(const PresetsUpdateReq &req, WriteCb<PresetsUpdateRes> cb)
{
    Data::Models::Presets::Update(
        m_db,
        Data::Models::Presets::Preset{
            .id = req.id,
            .name = req.name,
            .category = req.category,
            .download_limit = req.download_limit,
            .max_connections = req.max_connections,
            .max_uploads = req.max_uploads,
            .metadata = req.metadata,
            .session = req.session,
            .save_path = req.save_path,
            .storage_mode = req.storage_mode,
            .tags = req.tags,
            .upload_limit = req.upload_limit});

    cb.Ok(PresetsUpdateRes{});
}
