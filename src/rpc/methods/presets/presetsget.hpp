#pragma once

#include "../../typedmethod.hpp"

#include "presetsget_reqres.hpp"

namespace porla
{
    class Presets;
}

namespace porla::Rpc::Methods::Presets
{
    class PresetsGet : public TypedMethod<PresetsGetReq, PresetsGetRes>
    {
    public:
        explicit PresetsGet(porla::Presets& presets);

    protected:
        void Execute(const PresetsGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Presets& m_presets;
    };
}
