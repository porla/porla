#pragma once

#include "../config.hpp"
#include "method.hpp"
#include "presetslist_reqres.hpp"

namespace porla::Methods
{
    class PresetsList : public Method<PresetsListReq, PresetsListRes>
    {
    public:
        explicit PresetsList(const std::map<std::string, Config::Preset>& presets);

    protected:
        void Invoke(const PresetsListReq& req, WriteCb<PresetsListRes> cb) override;

    private:
        const std::map<std::string, Config::Preset>& m_presets;
    };
}
