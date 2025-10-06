#include "presetsremove.hpp"

#include "../../data/models/presets.hpp"

using porla::Methods::Presets::PresetsRemove;
using porla::Methods::Presets::PresetsRemoveReq;
using porla::Methods::Presets::PresetsRemoveRes;

PresetsRemove::PresetsRemove(sqlite3* db)
    : m_db(db)
{
}

void PresetsRemove::Invoke(const PresetsRemoveReq& req, WriteCb<PresetsRemoveRes> cb)
{
    const auto preset = Data::Models::Presets::GetById(m_db, req.id);

    if (!preset.has_value())
    {
        return cb.Error(-1, "Preset not found");
    }

    Data::Models::Presets::Remove(m_db, req.id);

    cb.Ok(PresetsRemoveRes{});
}
