#pragma once

#include "../../typedmethod.hpp"
#include "sessionsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsResume : public TypedMethod<SessionsResumeReq, SessionsResumeRes>
    {
    public:
        explicit SessionsResume(porla::Sessions& sessions);

    protected:
        void Execute(const SessionsResumeReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
