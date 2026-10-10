#pragma once

#include "../../typedmethod.hpp"
#include "sessionssettingsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsSettingsGet : public TypedMethod<SessionsSettingsGetReq, SessionsSettingsGetRes>
    {
    public:
        explicit SessionsSettingsGet(porla::Sessions& sessions);

    protected:
        void Execute(const SessionsSettingsGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
