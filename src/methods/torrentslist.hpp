#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "torrentslist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsList : public Method<TorrentsListReq, TorrentsListRes>
    {
    public:
        explicit TorrentsList(sqlite3* db, porla::ISession& session);

        void Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::ISession& m_session;
    };
}
