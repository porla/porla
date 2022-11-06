#include "presetslist.hpp"

using porla::Methods::PresetsList;
using porla::Methods::PresetsListReq;
using porla::Methods::PresetsListRes;

PresetsList::PresetsList(const std::map<std::string, Config::Preset>& presets)
    : m_presets(presets)
{
}

void PresetsList::Invoke(const PresetsListReq& req, WriteCb<PresetsListRes> cb)
{
    cb.Ok(PresetsListRes{
        .presets = m_presets
    });
}
