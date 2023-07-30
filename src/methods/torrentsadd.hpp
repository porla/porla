#pragma once

#include <sqlite3.h>

#include "../config.hpp"
#include "method.hpp"
#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsAdd : public Method<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(sqlite3* db, Sessions& session, const std::map<std::string, Config::Preset>& presets);

    protected:
        void Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb) override;

    private:
        sqlite3* m_db;
        Sessions& m_sessions;
        const std::map<std::string, Config::Preset>& m_presets;
    };
}
