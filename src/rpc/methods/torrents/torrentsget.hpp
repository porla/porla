#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsGet : public TypedMethod<TorrentsGetReq, TorrentsGetRes>
    {
    public:
        explicit TorrentsGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
