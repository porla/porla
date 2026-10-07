#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"

#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class Presets;
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsAdd : public TypedMethod<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(sqlite3* db, porla::Presets& presets, porla::Sessions& session);

    protected:
        void Execute(const TorrentsAddReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Presets& m_presets;
        porla::Sessions& m_sessions;
    };
}
