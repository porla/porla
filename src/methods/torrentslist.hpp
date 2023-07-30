#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "torrentslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsList : public Method<TorrentsListReq, TorrentsListRes>
    {
    public:
        explicit TorrentsList(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
