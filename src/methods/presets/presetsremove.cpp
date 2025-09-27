#include "presetsremove.hpp"

using porla::Methods::Presets::PresetsRemove;
using porla::Methods::Presets::PresetsRemoveReq;
using porla::Methods::Presets::PresetsRemoveRes;

PresetsRemove::PresetsRemove(sqlite3* db)
    : m_db(db)
{
}

void PresetsRemove::Invoke(const PresetsRemoveReq& req, WriteCb<PresetsRemoveRes> cb)
{
    cb.Ok(PresetsRemoveRes{});
}
