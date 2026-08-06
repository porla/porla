#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentspeerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPeersList : public Method<TorrentsPeersListReq, TorrentsPeersListRes>
    {
    public:
        explicit TorrentsPeersList(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const TorrentsPeersListReq& req, WriteCb<TorrentsPeersListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
