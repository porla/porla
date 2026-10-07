#pragma once

#include "../../typedmethod.hpp"

#include "presetsadd_reqres.hpp"

namespace porla
{
    class Presets;
}

namespace porla::Rpc::Methods::Presets
{
    class PresetsAdd : public TypedMethod<PresetsAddReq, PresetsAddRes>
    {
    public:
        explicit PresetsAdd(porla::Presets& presets);

    protected:
        void Execute(const PresetsAddReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Presets& m_presets;
    };
}
