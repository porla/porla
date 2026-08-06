#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentspeersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPeersAdd : public Method<TorrentsPeersAddReq, TorrentsPeersAddRes>
    {
    public:
        explicit TorrentsPeersAdd(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const TorrentsPeersAddReq& req, WriteCb<TorrentsPeersAddRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
