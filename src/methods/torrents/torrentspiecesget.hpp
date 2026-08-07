#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentspiecesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPiecesGet : public Method<TorrentsPiecesGetReq, TorrentsPiecesGetRes>
    {
    public:
        explicit TorrentsPiecesGet(sqlite3* db, porla::Sessions& sessions);

        void Invoke(const TorrentsPiecesGetReq& req, WriteCb<TorrentsPiecesGetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
