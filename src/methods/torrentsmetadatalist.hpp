#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "torrentsmetadatalist_reqres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsMetadataList : public Method<TorrentsMetadataListReq, TorrentsMetadataListRes>
    {
    public:
        explicit TorrentsMetadataList(sqlite3* db, ISession& session);

    protected:
        void Invoke(const TorrentsMetadataListReq& req, WriteCb<TorrentsMetadataListRes> cb) override;

    private:
        sqlite3* m_db;
        ISession& m_session;
    };
}
