#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsmigrate_reqres.hpp"

namespace porla
{
    class Events;
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsMigrate : public TypedMethod<TorrentsMigrateReq, TorrentsMigrateRes>, public std::enable_shared_from_this<TorrentsMigrate>
    {
    public:
        explicit TorrentsMigrate(sqlite3* db, porla::Events& events, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsMigrateReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Events& m_events;
        porla::Sessions& m_sessions;
    };
}
