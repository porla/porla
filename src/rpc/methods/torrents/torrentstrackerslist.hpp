#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentstrackerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsTrackersList : public TypedMethod<TorrentsTrackersListReq, TorrentsTrackersListRes>
    {
    public:
        explicit TorrentsTrackersList(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsTrackersListReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
