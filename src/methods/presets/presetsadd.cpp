#include "presetsadd.hpp"

namespace fs = std::filesystem;

using porla::Methods::Presets::PresetsAdd;
using porla::Methods::Presets::PresetsAddReq;
using porla::Methods::Presets::PresetsAddRes;

PresetsAdd::PresetsAdd(sqlite3* db)
    : m_db(db)
{
}

void PresetsAdd::Invoke(const PresetsAddReq& req, WriteCb<PresetsAddRes> cb)
{
    cb.Ok(PresetsAddRes{});
}
