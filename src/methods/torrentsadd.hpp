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
    template <bool SSL> class TorrentsAdd : public Method<TorrentsAddReq, TorrentsAddRes, SSL>
    {
    public:
        explicit TorrentsAdd(Sessions& session, const std::map<std::string, Config::Preset>& presets);

    protected:
        void Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
        const std::map<std::string, Config::Preset>& m_presets;
    };
}
