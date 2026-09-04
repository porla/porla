#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentspeerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPeersList : public TypedMethod<TorrentsPeersListReq, TorrentsPeersListRes>
    {
    public:
        explicit TorrentsPeersList(sqlite3* db, porla::Sessions& sessions);

        void Execute(const TorrentsPeersListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
