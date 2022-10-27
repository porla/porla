#pragma once

#include <toml++/toml.h>

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
        explicit TorrentsAdd(ISession& session, toml::table& cfg);

    protected:
        void Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb) override;

    private:
        toml::table& m_cfg;
        ISession& m_session;
    };
}
