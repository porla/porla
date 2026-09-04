#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsrecheck_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsRecheck : public TypedMethod<TorrentsRecheckReq, TorrentsRecheckRes>
    {
    public:
        explicit TorrentsRecheck(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsRecheckReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
