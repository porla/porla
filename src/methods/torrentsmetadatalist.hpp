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
    template <bool SSL> class TorrentsMetadataList : public Method<TorrentsMetadataListReq, TorrentsMetadataListRes, SSL>
    {
    public:
        explicit TorrentsMetadataList(sqlite3* db, Sessions& sessions);

    protected:
        void Invoke(const TorrentsMetadataListReq& req, WriteCb<TorrentsMetadataListRes, SSL> cb) override;

    private:
        sqlite3* m_db;
        Sessions& m_sessions;
    };
}
