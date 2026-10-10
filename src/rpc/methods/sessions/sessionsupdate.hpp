#pragma once

#include "../../typedmethod.hpp"
#include "sessionsupdate_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsUpdate : public TypedMethod<SessionsUpdateReq, SessionsUpdateRes>
    {
    public:
        explicit SessionsUpdate(porla::Sessions& sessions);

        void Execute(const SessionsUpdateReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
