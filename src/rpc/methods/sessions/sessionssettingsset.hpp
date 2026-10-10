#pragma once

#include "../../typedmethod.hpp"
#include "sessionssettingsset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsSettingsSet : public TypedMethod<SessionsSettingsSetReq, SessionsSettingsSetRes>
    {
    public:
        explicit SessionsSettingsSet(porla::Sessions& sessions);

    protected:
        void Execute(const SessionsSettingsSetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
