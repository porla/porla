#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsGet : public Method<TorrentsGetReq, TorrentsGetRes>
    {
    public:
        explicit TorrentsGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsGetReq& req, WriteCb<TorrentsGetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
