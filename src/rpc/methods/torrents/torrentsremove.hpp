#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsRemove : public TypedMethod<TorrentsRemoveReq, TorrentsRemoveRes>
    {
    public:
        explicit TorrentsRemove(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsRemoveReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
