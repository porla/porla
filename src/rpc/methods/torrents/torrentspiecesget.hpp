#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentspiecesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPiecesGet : public TypedMethod<TorrentsPiecesGetReq, TorrentsPiecesGetRes>
    {
    public:
        explicit TorrentsPiecesGet(sqlite3* db, porla::Sessions& sessions);

        void Execute(const TorrentsPiecesGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
