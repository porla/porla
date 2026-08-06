#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsResume : public Method<TorrentsResumeReq, TorrentsResumeRes>
    {
    public:
        explicit TorrentsResume(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsResumeReq& req, WriteCb<TorrentsResumeRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
