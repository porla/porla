#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPause : public Method<TorrentsPauseReq, TorrentsPauseRes>
    {
    public:
        explicit TorrentsPause(sqlite3* db, porla::Sessions& session);

    protected:
        void Invoke(const TorrentsPauseReq& req, WriteCb<TorrentsPauseRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
