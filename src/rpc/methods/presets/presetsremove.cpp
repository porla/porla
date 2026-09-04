#include "presetsremove.hpp"

#include "../../../data/models/presets.hpp"

using porla::Rpc::Methods::Presets::PresetsRemove;
using porla::Rpc::Methods::Presets::PresetsRemoveReq;
using porla::Rpc::Methods::Presets::PresetsRemoveRes;

PresetsRemove::PresetsRemove(sqlite3* db)
    : m_db(db)
{
}

void PresetsRemove::Execute(const PresetsRemoveReq& req, ResponseWriterHandle cb)
{
    const auto preset = Data::Models::Presets::GetById(m_db, req.id);

    if (!preset.has_value())
    {
        return cb->Error(-1, "Preset not found");
    }

    Data::Models::Presets::Remove(m_db, req.id);

    cb->Ok(PresetsRemoveRes{});
}
