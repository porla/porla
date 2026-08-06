#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentspropertiesget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsPropertiesGet : public Method<TorrentsPropertiesGetReq, TorrentsPropertiesGetRes>
    {
    public:
        explicit TorrentsPropertiesGet(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsPropertiesGetReq& req, WriteCb<TorrentsPropertiesGetRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
