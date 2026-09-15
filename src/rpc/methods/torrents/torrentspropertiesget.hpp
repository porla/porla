#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentspropertiesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPropertiesGet : public TypedMethod<TorrentsPropertiesGetReq, TorrentsPropertiesGetRes>
    {
    public:
        explicit TorrentsPropertiesGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsPropertiesGetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
