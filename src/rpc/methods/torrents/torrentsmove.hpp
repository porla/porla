#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsmove_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsMove : public TypedMethod<TorrentsMoveReq, TorrentsMoveRes>
    {
    public:
        explicit TorrentsMove(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsMoveReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
