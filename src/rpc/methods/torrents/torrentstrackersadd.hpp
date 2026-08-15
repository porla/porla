#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentstrackersadd_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsTrackersAdd : public TypedMethod<TorrentsTrackersAddReq, TorrentsTrackersAddRes>
    {
    public:
        explicit TorrentsTrackersAdd(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsTrackersAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
