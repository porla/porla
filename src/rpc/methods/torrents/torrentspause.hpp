#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentspause_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPause : public TypedMethod<TorrentsPauseReq, TorrentsPauseRes>
    {
    public:
        explicit TorrentsPause(sqlite3* db, porla::Sessions& session);

    protected:
        void Execute(const TorrentsPauseReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
