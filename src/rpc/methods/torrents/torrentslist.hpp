#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsList : public TypedMethod<TorrentsListReq, TorrentsListRes>
    {
    public:
        explicit TorrentsList(sqlite3* db, porla::Sessions& sessions);

        void Execute(const TorrentsListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
