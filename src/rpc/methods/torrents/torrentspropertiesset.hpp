#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentspropertiesset_reqres.hpp"

namespace porla
{
    class Sessions;
    class Torrents;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsPropertiesSet : public TypedMethod<TorrentsPropertiesSetReq, TorrentsPropertiesSetRes>
    {
    public:
        explicit TorrentsPropertiesSet(sqlite3* db, porla::Sessions& sessions, porla::Torrents& torrents);

    protected:
        void Execute(const TorrentsPropertiesSetReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
        porla::Torrents& m_torrents;
    };
}
