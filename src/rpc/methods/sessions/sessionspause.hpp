#pragma once

#include "../../typedmethod.hpp"
#include "sessionref.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsPause : public TypedMethod<SessionRef>
    {
    public:
        explicit SessionsPause(porla::Sessions& sessions);

    protected:
        void Execute(const SessionRef& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
