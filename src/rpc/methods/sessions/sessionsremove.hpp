#pragma once

#include "../../typedmethod.hpp"
#include "sessionsremove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsRemove : public TypedMethod<SessionsRemoveReq, SessionsRemoveRes>
    {
    public:
        explicit SessionsRemove(porla::Sessions& sessions);

        void Execute(const SessionsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
