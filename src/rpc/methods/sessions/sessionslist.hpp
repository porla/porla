#pragma once

#include "../../typedmethod.hpp"
#include "sessionslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsList : public TypedMethod<SessionsListReq, SessionsListRes>
    {
    public:
        explicit SessionsList(porla::Sessions& sessions);

        void Execute(const SessionsListReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
