#pragma once

#include "../../typedmethod.hpp"
#include "sessionsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsGet : public TypedMethod<SessionsGetReq, SessionsGetRes>
    {
    public:
        explicit SessionsGet(porla::Sessions& sessions);

    protected:
        void Execute(const SessionsGetReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
