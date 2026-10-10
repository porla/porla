#pragma once

#include "../../typedmethod.hpp"
#include "sessionspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsPause : public TypedMethod<SessionsPauseReq, SessionsPauseRes>
    {
    public:
        explicit SessionsPause(porla::Sessions& sessions);

    protected:
        void Execute(const SessionsPauseReq& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
