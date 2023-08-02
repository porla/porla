#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "sessionslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class SessionsList : public Method<SessionsListReq, SessionsListRes>
    {
    public:
        explicit SessionsList(porla::Sessions& sessions);

        void Invoke(const SessionsListReq& req, WriteCb<SessionsListRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
