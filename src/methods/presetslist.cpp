#include "presetslist.hpp"

using porla::Methods::PresetsList;
using porla::Methods::PresetsListReq;
using porla::Methods::PresetsListRes;

template <bool SSL> PresetsList<SSL>::PresetsList(const std::map<std::string, Config::Preset>& presets)
    : m_presets(presets)
{
}

template <bool SSL> void PresetsList<SSL>::Invoke(const PresetsListReq& req, WriteCb<PresetsListRes, SSL> cb)
{
    cb.Ok(PresetsListRes{
        .presets = m_presets
    });
}

namespace porla::Methods {
    template class PresetsList<true>;
    template class PresetsList<false>;
}
