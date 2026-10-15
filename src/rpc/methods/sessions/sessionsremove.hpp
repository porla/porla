#pragma once

#include "../../typedmethod.hpp"
#include "sessionref.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Sessions
{
    class SessionsRemove : public TypedMethod<SessionRef>
    {
    public:
        explicit SessionsRemove(porla::Sessions& sessions);

        void Execute(const SessionRef& req, ResponseWriterHandle cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
