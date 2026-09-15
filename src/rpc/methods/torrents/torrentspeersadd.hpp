#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentspeersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPeersAdd : public TypedMethod<TorrentsPeersAddReq, TorrentsPeersAddRes>
    {
    public:
        explicit TorrentsPeersAdd(sqlite3* db, porla::Sessions& sessions);

        void Execute(const TorrentsPeersAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
