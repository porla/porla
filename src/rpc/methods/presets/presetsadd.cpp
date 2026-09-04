#include "presetsadd.hpp"

#include "../../../data/models/presets.hpp"

using porla::Rpc::Methods::Presets::PresetsAdd;
using porla::Rpc::Methods::Presets::PresetsAddReq;
using porla::Rpc::Methods::Presets::PresetsAddRes;

PresetsAdd::PresetsAdd(sqlite3* db)
    : m_db(db)
{
}

void PresetsAdd::Execute(const PresetsAddReq& req, ResponseWriterHandle cb)
{
    auto preset_id = Data::Models::Presets::Insert(
        m_db,
        req.name);

    cb->Ok(PresetsAddRes{
        .id = preset_id
    });
}
