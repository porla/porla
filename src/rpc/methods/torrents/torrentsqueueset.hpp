#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsqueueset_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsQueueSet : public TypedMethod<TorrentsQueueSetReq, TorrentsQueueSetRes>
    {
    public:
        explicit TorrentsQueueSet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsQueueSetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
