#pragma once

#include "../../typedmethod.hpp"

#include "presetsupdate_reqres.hpp"

namespace porla
{
    class Presets;
}

namespace porla::Rpc::Methods::Presets
{
    class PresetsUpdate : public TypedMethod<PresetsUpdateReq, PresetsUpdateRes>
    {
    public:
        explicit PresetsUpdate(porla::Presets& presets);

    protected:
        void Execute(const PresetsUpdateReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Presets& m_presets;
    };
}
