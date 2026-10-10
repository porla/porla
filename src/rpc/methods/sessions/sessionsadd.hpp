#pragma once

#include "../../typedmethod.hpp"
#include "sessionsadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsAdd : public TypedMethod<SessionsAddReq, SessionsAddRes>
    {
    public:
        explicit SessionsAdd(porla::Sessions& sessions);

        void Execute(const SessionsAddReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
