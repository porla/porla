#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentsmove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsMove : public Method<TorrentsMoveReq, TorrentsMoveRes>
    {
    public:
        explicit TorrentsMove(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsMoveReq& req, WriteCb<TorrentsMoveRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
