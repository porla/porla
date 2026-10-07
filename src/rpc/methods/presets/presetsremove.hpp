#pragma once

#include "../../typedmethod.hpp"
#include "presetsremove_reqres.hpp"

namespace porla
{
    class Presets;
}

namespace porla::Rpc::Methods::Presets
{
    class PresetsRemove : public TypedMethod<PresetsRemoveReq, PresetsRemoveRes>
    {
    public:
        explicit PresetsRemove(porla::Presets& presets);

    protected:
        void Execute(const PresetsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Presets& m_presets;
    };
}
