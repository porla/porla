#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsAdd : public TypedMethod<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(sqlite3* db, porla::Sessions& session);

    protected:
        void Execute(const TorrentsAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
