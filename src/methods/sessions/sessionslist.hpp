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
    template <bool SSL> class SessionsList : public Method<SessionsListReq, SessionsListRes, SSL>
    {
    public:
        explicit SessionsList(porla::Sessions& sessions);

        void Invoke(const SessionsListReq& req, WriteCb<SessionsListRes, SSL> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
