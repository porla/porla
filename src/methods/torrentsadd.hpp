#pragma once

#include <toml++/toml.h>

#include "../config.hpp"
#include "method.hpp"
#include "torrentsaddreq.hpp"
#include "torrentsaddres.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Methods
{
    class TorrentsAdd : public Method<TorrentsAddReq, TorrentsAddRes>
    {
    public:
        explicit TorrentsAdd(ISession& session, const std::map<std::string, Config::Preset>& presets);

    protected:
        void Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb) override;

    private:
        ISession& m_session;
        const std::map<std::string, Config::Preset>& m_presets;
    };
}
