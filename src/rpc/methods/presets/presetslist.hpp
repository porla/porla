#pragma once

#include "../../typedmethod.hpp"

#include "presetslist_reqres.hpp"

namespace porla
{
    class Presets;
}

namespace porla::Rpc::Methods::Presets
{
    class PresetsList : public TypedMethod<PresetsListReq, PresetsListRes>
    {
    public:
        explicit PresetsList(porla::Presets& presets);

    protected:
        void Execute(const PresetsListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Presets& m_presets;
    };
}
