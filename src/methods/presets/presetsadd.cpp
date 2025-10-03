#include "presetsadd.hpp"

#include "../../data/models/presets.hpp"

using porla::Methods::Presets::PresetsAdd;
using porla::Methods::Presets::PresetsAddReq;
using porla::Methods::Presets::PresetsAddRes;

PresetsAdd::PresetsAdd(sqlite3* db)
    : m_db(db)
{
}

void PresetsAdd::Invoke(const PresetsAddReq& req, WriteCb<PresetsAddRes> cb)
{
    auto preset_id = Data::Models::Presets::Insert(
        m_db,
        req.name);

    cb.Ok(PresetsAddRes{
        .id = preset_id
    });
}
