#include "presetsadd.hpp"

#include "../../../presets.hpp"

using porla::Rpc::Methods::Presets::PresetsAdd;
using porla::Rpc::Methods::Presets::PresetsAddReq;
using porla::Rpc::Methods::Presets::PresetsAddRes;

PresetsAdd::PresetsAdd(porla::Presets& presets)
    : m_presets(presets)
{
}

void PresetsAdd::Execute(const PresetsAddReq& req, ResponseWriterHandle cb)
{
    cb->Ok(PresetsAddRes{
        .id = m_presets.Add(req.name)
    });
}
