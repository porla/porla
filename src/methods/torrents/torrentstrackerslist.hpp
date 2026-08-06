#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentstrackerslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsTrackersList : public Method<TorrentsTrackersListReq, TorrentsTrackersListRes>
    {
    public:
        explicit TorrentsTrackersList(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsTrackersListReq& req, WriteCb<TorrentsTrackersListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
