#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "torrentsmetadatalist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsMetadataList : public Method<TorrentsMetadataListReq, TorrentsMetadataListRes>
    {
    public:
        explicit TorrentsMetadataList(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsMetadataListReq& req, WriteCb<TorrentsMetadataListRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
